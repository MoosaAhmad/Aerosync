#include "UserDashboard.h"
#include "LoginWindow.h"
#include "Booking.h"
#include "Payment.h"
#include "Receipt.h"
#include "Seat.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QMessageBox>
#include <QDialog>
#include <QScrollArea>
#include <QDateTime>
#include <QMenu>
#include <QAction>
#include <cmath>

UserDashboard::UserDashboard(user* currentUser, AppContext* ctx, QWidget* parent)
    : QMainWindow(parent), currentUser(currentUser), ctx(ctx)
{
    setWindowTitle("AeroSync – " +
                   QString::fromStdString(currentUser->getname()));
    resize(1200, 720);

    auto* central = new QWidget;
    auto* mainLay = new QHBoxLayout(central);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);
    setCentralWidget(central);

    sidebar = new QListWidget;
    sidebar->setFixedWidth(230);
    sidebar->setObjectName("sidebar");
    sidebar->addItems({
        "  🔍  Search Flights",
        "  📋  My Bookings",
        "  💳  Payment",
        "  🧾  Receipt",
        "  🔒  Change Password",
        "  🚪  Logout"
    });
    sidebar->setCurrentRow(0);
    connect(sidebar, &QListWidget::currentRowChanged,
            this, &UserDashboard::onSidebarClicked);

    contentStack = new QStackedWidget;
    contentStack->addWidget(buildSearchPage());   // 0
    contentStack->addWidget(buildBookingsPage()); // 1
    contentStack->addWidget(buildPaymentPage());  // 2
    contentStack->addWidget(buildReceiptPage());  // 3

    mainLay->addWidget(sidebar);
    mainLay->addWidget(contentStack, 1);

    applyStyles();
}

void UserDashboard::onSidebarClicked(int row) {
    if (row == 5) { logout(); return; }
    if (row == 4) { openChangePassword(); return; }
    if (row == 0) searchFlights();
    if (row == 1) loadMyBookings();
    contentStack->setCurrentIndex(row);
}

void UserDashboard::openChangePassword()
{
    int prevPage = contentStack->currentIndex();

    ChangePasswordDialog dlg(currentUser,ctx, this);
    if (dlg.exec() == QDialog::Accepted) {
        QMessageBox::information(this, "Password Updated",
                                 "✅  Your password has been changed successfully.");
    }

    sidebar->setCurrentRow(prevPage);
    contentStack->setCurrentIndex(prevPage);
}

// ── Search Flights ─────────────────────────────────────────────────────────
QWidget* UserDashboard::buildSearchPage() {
    auto* w = new QWidget;
    auto* v = new QVBoxLayout(w);
    v->setContentsMargins(20, 20, 20, 20);
    v->setSpacing(10);

    auto* title = new QLabel("Search & Book Flights");
    title->setObjectName("pageTitle");
    v->addWidget(title);

    // ── Filter row ─────────────────────────────────────────────────────────
    auto* filterRow = new QHBoxLayout;

    searchFrom = new QLineEdit;
    searchFrom->setPlaceholderText("From (e.g. Lahore)");

    searchTo = new QLineEdit;
    searchTo->setPlaceholderText("To (e.g. Dubai)");

    // Date filter: checkbox + QDateEdit with calendar popup
    dateFilterCheck = new QCheckBox("Date:");
    dateFilterCheck->setCursor(Qt::PointingHandCursor);

    searchDate = new QDateEdit(QDate::currentDate());
    searchDate->setDisplayFormat("dd-MM-yyyy");
    searchDate->setCalendarPopup(true);
    searchDate->setEnabled(false); // disabled until checkbox ticked

    // Calendar emoji on the popup button
    if (auto* calBtn = searchDate->findChild<QAbstractButton*>())
        calBtn->setText("v");

    connect(dateFilterCheck, &QCheckBox::toggled,
            searchDate, &QDateEdit::setEnabled);

    auto* searchBtn = new QPushButton("🔍  Search");
    searchBtn->setObjectName("actionBtn");
    searchBtn->setCursor(Qt::PointingHandCursor);
    connect(searchBtn, &QPushButton::clicked, this, &UserDashboard::searchFlights);

    filterRow->addWidget(searchFrom);
    filterRow->addWidget(searchTo);
    filterRow->addWidget(dateFilterCheck);
    filterRow->addWidget(searchDate);
    filterRow->addWidget(searchBtn);
    v->addLayout(filterRow);

    // ── Results table ───────────────────────────────────────────────────────
    searchTable = new QTableWidget(0, 7);
    searchTable->setHorizontalHeaderLabels(
        {"Flight ID","From","To","Departure","Arrival","Status","Avail. Seats"});
    searchTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    searchTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    searchTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    searchTable->setAlternatingRowColors(true);
    connect(searchTable, &QTableWidget::cellDoubleClicked,
            this, &UserDashboard::onFlightDoubleClicked);
    v->addWidget(searchTable);

    searchHint = new QLabel("💡 Double-click a flight row to choose a seat and book");
    searchHint->setObjectName("infoLabel");
    v->addWidget(searchHint);

    searchFlights();
    return w;
}

void UserDashboard::searchFlights() {
    searchTable->setRowCount(0);

    std::string from = searchFrom->text().trimmed().toStdString();
    std::string to   = searchTo->text().trimmed().toStdString();

    for (auto* f : ctx->flightRepo.share()) {
        if (f->getStatus() != Flight::FlightStatus::Scheduled) continue;
        if (!from.empty() &&
            f->getSource().find(from) == std::string::npos) continue;
        if (!to.empty() &&
            f->getDestination().find(to) == std::string::npos) continue;

        // Date filter — only active when checkbox is ticked
        if (dateFilterCheck->isChecked()) {
            try {
                std::string dtStr = searchDate->date()
                .toString("dd-MM-yyyy").toStdString();
                date filter = date::fromString(dtStr);
                if (!(f->getDeparture().get_date() == filter)) continue;
            } catch (...) {}
        }

        int r = searchTable->rowCount();
        searchTable->insertRow(r);
        int avail = (int)f->getAvailableSeats().size();

        searchTable->setItem(r,0,new QTableWidgetItem(QString::fromStdString(f->getId())));
        searchTable->setItem(r,1,new QTableWidgetItem(QString::fromStdString(f->getSource())));
        searchTable->setItem(r,2,new QTableWidgetItem(QString::fromStdString(f->getDestination())));
        searchTable->setItem(r,3,new QTableWidgetItem(QString::fromStdString(datetime::toString(f->getDeparture()))));
        searchTable->setItem(r,4,new QTableWidgetItem(QString::fromStdString(datetime::toString(f->getArrival()))));
        searchTable->setItem(r,5,new QTableWidgetItem(QString::fromStdString(Flight::statusToString(f->getStatus()))));
        searchTable->setItem(r,6,new QTableWidgetItem(QString::number(avail)));
    }

    searchHint->setText(searchTable->rowCount() == 0
                            ? "❌ No scheduled flights found."
                            : "💡 Double-click a flight row to choose a seat and book");
}

void UserDashboard::onFlightDoubleClicked(int row, int /*col*/) {
    QString id = searchTable->item(row, 0)->text();
    Flight* f  = ctx->flightRepo.getFlightById(id.toStdString());
    if (f) showSeatPicker(f);
}

void UserDashboard::showSeatPicker(Flight* flight) {
    auto seats = flight->getAvailableSeats();
    if (seats.empty()) {
        QMessageBox::information(this, "Fully Booked",
                                 "No seats available on this flight.");
        return;
    }

    QDialog dlg(this);
    dlg.setWindowTitle("Pick a Seat  –  " + QString::fromStdString(flight->getId()));
    dlg.setMinimumSize(540, 420);
    // CHANGED: dialog background from deep navy (#0d0d1a) to deep dark brown (#0d0a06)
    dlg.setStyleSheet(
        "QDialog{background:#0d0a06;color:#d4c5a9;"
        "font-family:'Garamond','Times New Roman',serif;font-size:13px;}"
        "QScrollArea{border:none;background:#110e08;}"
        "QWidget#scrollContent{background:#110e08;}");

    auto* vMain = new QVBoxLayout(&dlg);

    // CHANGED: flight info accent from #4fc3f7 (cyan) to #f97316 (orange)
    auto* info = new QLabel(QString(
                                "<b style='color:#f97316;font-size:15px;'>%1 → %2</b>"
                                "<span style='color:#6b5a40;'> &nbsp;|&nbsp; %3</span>")
                                .arg(QString::fromStdString(flight->getSource()))
                                .arg(QString::fromStdString(flight->getDestination()))
                                .arg(QString::fromStdString(datetime::toString(flight->getDeparture()))));
    info->setTextFormat(Qt::RichText);
    vMain->addWidget(info);

    auto* legend = new QLabel(
        "  <span style='color:#fb923c;'>🟧 Economy</span>"
        "  &nbsp;&nbsp;"
        "  <span style='color:#a78bfa;'>🟪 Business</span>"
        "  &nbsp;&nbsp; (price in PKR)");
    legend->setTextFormat(Qt::RichText);
    vMain->addWidget(legend);

    auto* scrollContent = new QWidget;
    scrollContent->setObjectName("scrollContent");
    auto* grid = new QGridLayout(scrollContent);
    grid->setSpacing(7);

    int ecoCount = 0;
    for (auto* s : seats)
        if (s->getSeatClass() == Seat::SeatClass::Economy) ecoCount++;

    int ecoGridRows = (ecoCount + 4) / 5;
    int bizStartRow = ecoGridRows + 2;

    if (ecoCount > 0) {
        auto* eH = new QLabel("Economy");
        eH->setStyleSheet("color:#fb923c;font-weight:bold;font-size:12px;");
        grid->addWidget(eH, 0, 0, 1, 5);
    }

    int ei = 0, bi = 0;
    bool bizLabelAdded = false;

    for (auto* s : seats) {
        bool isBiz = (s->getSeatClass() == Seat::SeatClass::Business);
        std::string seatNo = s->getSeatNumber();
        double price = s->getPrice();

        auto* btn = new QPushButton(
            QString::fromStdString(seatNo) + "\n" +
            QString::number(price / 1000.0, 'f', 0) + "k");
        btn->setFixedSize(85, 56);
        btn->setCursor(Qt::PointingHandCursor);
        // Business seat buttons: kept purple (unchanged per request)
        // Economy seat buttons: kept orange (unchanged per request)
        btn->setStyleSheet(isBiz
                               ? "QPushButton{background:#1a1a3a;color:#a78bfa;border:1px solid #4c3a9e;"
                                 "border-radius:6px;font-size:11px;font-weight:bold;}"
                                 "QPushButton:hover{background:#2d1f6e;color:#c4b5fd;border:2px solid #7c5cf7;}"
                                 "QPushButton:pressed{background:#3b28a0;border:2px solid #a78bfa;}"
                               : "QPushButton{background:#221a0e;color:#fb923c;border:1px solid #7c3a0e;"
                                 "border-radius:6px;font-size:11px;font-weight:bold;}"
                                 "QPushButton:hover{background:#431d05;color:#fdba74;border:2px solid #f97316;}"
                                 "QPushButton:pressed{background:#5a2500;border:2px solid #fb923c;}");

        connect(btn, &QPushButton::clicked,
                [=, &dlg, isBusiness = isBiz]()
                {
                    pendingFlightId = flight->getId();
                    pendingSeatNo   = seatNo;
                    pendingPrice    = price;

                    // CHANGED: accent color in summary from #4fc3f7 (cyan) to #f97316 (orange)
                    paymentSummary->setText(QString(
                                                "<b>Flight:</b> %1 &nbsp; %2 → %3<br>"
                                                "<b>Departure:</b> %4<br>"
                                                "<b>Seat:</b> %5 &nbsp;&nbsp; <b>Class:</b> %6<br>"
                                                "<b>Amount:</b> <span style='color:#f97316;'>PKR %7</span>")
                                                .arg(QString::fromStdString(flight->getId()))
                                                .arg(QString::fromStdString(flight->getSource()))
                                                .arg(QString::fromStdString(flight->getDestination()))
                                                .arg(QString::fromStdString(datetime::toString(flight->getDeparture())))
                                                .arg(QString::fromStdString(seatNo))
                                                .arg(isBusiness ? "Business" : "Economy")
                                                .arg(price, 0, 'f', 2));

                    sidebar->setCurrentRow(2);
                    contentStack->setCurrentIndex(2);
                    dlg.accept();
                });

        if (!isBiz) {
            grid->addWidget(btn, 1 + ei / 5, ei % 5);
            ei++;
        } else {
            if (!bizLabelAdded) {
                auto* bH = new QLabel("Business");
                bH->setStyleSheet("color:#a78bfa;font-weight:bold;font-size:12px;");
                grid->addWidget(bH, bizStartRow, 0, 1, 5);
                bizLabelAdded = true;
            }
            grid->addWidget(btn, bizStartRow + 1 + bi / 5, bi % 5);
            bi++;
        }
    }

    auto* scroll = new QScrollArea;
    scroll->setWidget(scrollContent);
    scroll->setWidgetResizable(true);
    vMain->addWidget(scroll);

    // CHANGED: cancel button from navy (#151c30) to dark brown (#1a1208)
    auto* cancelBtn = new QPushButton("Cancel");
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet(
        "QPushButton{background:#1a1208;color:#8a7560;border:1px solid #3a2a10;"
        "border-radius:6px;padding:8px;}"
        "QPushButton:hover{background:#241a0a;color:#b09070;border:1px solid #6a4a20;}");
    connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);
    vMain->addWidget(cancelBtn);

    dlg.exec();
}

// ── My Bookings ────────────────────────────────────────────────────────────
QWidget* UserDashboard::buildBookingsPage() {
    auto* w = new QWidget;
    auto* v = new QVBoxLayout(w);
    v->setContentsMargins(20, 20, 20, 20);
    v->setSpacing(10);

    auto* title = new QLabel("My Bookings");
    title->setObjectName("pageTitle");
    v->addWidget(title);

    myBookingsTable = new QTableWidget(0, 6);
    myBookingsTable->setHorizontalHeaderLabels(
        { "Booking ID","Flight","From","To","Seat","Status" });
    myBookingsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    myBookingsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    myBookingsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    myBookingsTable->setAlternatingRowColors(true);

    // Double-click → show receipt dialog
    connect(myBookingsTable, &QTableWidget::cellDoubleClicked,
            [this](int row, int /*col*/) {
                QTableWidgetItem* item = myBookingsTable->item(row, 0);
                if (!item) return;
                Booking* b = reinterpret_cast<Booking*>(
                    item->data(Qt::UserRole).value<quintptr>());
                if (!b) return;

                bool cancelled = (b->getStatus() == Booking::BookingStatus::Cancelled);

                Flight* f = ctx->flightRepo.getFlightById(b->getFlightId());
                QString from    = f ? QString::fromStdString(f->getSource())      : "—";
                QString to      = f ? QString::fromStdString(f->getDestination()) : "—";
                QString dep     = f ? QString::fromStdString(datetime::toString(f->getDeparture())) : "—";
                QString arr     = f ? QString::fromStdString(datetime::toString(f->getArrival()))   : "—";

                QString seatNo  = QString::fromStdString(b->getSeatNumber());
                bool    isBiz   = !seatNo.isEmpty() && seatNo[0] == 'B';

                // Look up payment info via receipt → payment chain
                QString bank="—", method="—", maskedAcc="—", txnId="—",
                    receiptId="—", bookingDate="—";
                double  amount = 0.0;
                Receipt* receipt = ctx->receiptRepo.searchByBookingId(b->getBookingId());
                if (receipt) {
                    receiptId = QString::fromStdString(receipt->getReceiptId());
                    txnId     = QString::fromStdString(receipt->getPaymentId());
                    Payment* pay = ctx->paymentRepo.getPaymentById(txnId.toStdString());
                    if (pay) {
                        bank      = QString::fromStdString(pay->getBankName());
                        method    = QString::fromStdString(pay->getPaymentMethod());
                        amount    = pay->getAmount();
                        QString acc = QString::fromStdString(pay->getAccountNumber());
                        maskedAcc = acc.length() > 4
                                        ? QString("*").repeated(acc.length()-4) + acc.right(4)
                                        : acc;
                    }
                }
                bookingDate = QString::fromStdString(
                    datetime::toString(b->getBookingDate()));

                // Status-dependent labels
                QString statusColor  = cancelled ? "#f87171" : "#4ade80";
                QString statusIcon   = cancelled ? "❌" : "✅";
                QString statusText   = cancelled ? "Booking Cancelled — Payment Refunded"
                                               : "Payment Successful — Booking Confirmed";
                QString statusBg     = cancelled ? "#2d0707" : "#052e16";
                QString statusBorder = cancelled ? "#991b1b" : "#166534";
                QString bookingLabel = cancelled ? "Cancelled" : "Booked";
                QString payLabel     = cancelled ? "Refunded"  : "Paid";
                QString payColor     = cancelled ? "#f87171"   : "#4ade80";

                QString html = QString(R"(
<div style='font-family:Segoe UI,sans-serif; padding:10px; background:#141414;'>

  <!-- Header -->
  <div style='background:#0e0e0e; border:1px solid #1f2937;
              border-radius:12px; padding:20px;
              text-align:center; margin-bottom:16px;'>
    <div style='color:#f97316; font-size:24px;
                font-weight:bold; margin:6px 0;'>✈ AeroSync</div>
    <div style='color:#6b7280; font-size:11px;
                letter-spacing:3px;'>BOOKING RECEIPT</div>
  </div>

  <!-- Status badge -->
  <div style='background:%1; border:1px solid %2;
              border-radius:8px; padding:10px 16px;
              text-align:center; margin-bottom:16px;'>
    <span style='color:%3; font-size:14px; font-weight:bold;'>
      %4 %5
    </span>
  </div>

  <!-- Two column cards -->
  <table width='100%%' cellspacing='8' cellpadding='0'>
  <tr>

    <!-- Flight Info -->
    <td width='50%%' valign='top'>
      <div style='background:#0e0e0e; border:1px solid #1f2937;
                  border-radius:10px; padding:14px;'>
        <div style='color:#f97316; font-weight:bold;
                    font-size:13px; margin-bottom:10px;
                    border-bottom:1px solid #1f2937; padding-bottom:6px;'>
          ✈ Flight Details
        </div>
        <table cellpadding='5' width='100%%'>
          <tr><td style='color:#6b7280;'>Flight ID</td>
              <td style='color:#e5e7eb; font-weight:bold;'>%6</td></tr>
          <tr><td style='color:#6b7280;'>Route</td>
              <td style='color:#e5e7eb;'>%7 to %8</td></tr>
          <tr><td style='color:#6b7280;'>Departure</td>
              <td style='color:#e5e7eb;'>%9</td></tr>
          <tr><td style='color:#6b7280;'>Arrival</td>
              <td style='color:#e5e7eb;'>%10</td></tr>
          <tr><td style='color:#6b7280;'>Seat</td>
              <td style='color:#e5e7eb;'>%11</td></tr>
          <tr><td style='color:#6b7280;'>Class</td>
              <td style='color:%12; font-weight:bold;'>%13</td></tr>
          <tr><td style='color:#6b7280;'>Booked On</td>
              <td style='color:#e5e7eb;'>%14</td></tr>
          <tr><td style='color:#6b7280;'>Status</td>
              <td style='color:%3; font-weight:bold;'>%15</td></tr>
        </table>
      </div>
    </td>

    <!-- Payment Info -->
    <td width='50%%' valign='top'>
      <div style='background:#0e0e0e; border:1px solid #1f2937;
                  border-radius:10px; padding:14px;'>
        <div style='color:#f97316; font-weight:bold;
                    font-size:13px; margin-bottom:10px;
                    border-bottom:1px solid #1f2937; padding-bottom:6px;'>
          💳 Payment Details
        </div>
        <table cellpadding='5' width='100%%'>
          <tr><td style='color:#6b7280;'>Receipt ID</td>
              <td style='color:#e5e7eb;'>%16</td></tr>
          <tr><td style='color:#6b7280;'>Booking ID</td>
              <td style='color:#e5e7eb;'>%17</td></tr>
          <tr><td style='color:#6b7280;'>Transaction</td>
              <td style='color:#e5e7eb;'>%18</td></tr>
          <tr><td style='color:#6b7280;'>Bank</td>
              <td style='color:#e5e7eb;'>%19</td></tr>
          <tr><td style='color:#6b7280;'>Method</td>
              <td style='color:#e5e7eb;'>%20</td></tr>
          <tr><td style='color:#6b7280;'>Account</td>
              <td style='color:#e5e7eb;'>%21</td></tr>
          <tr><td style='color:#6b7280;'>Payment</td>
              <td style='color:%22; font-weight:bold;'>%23</td></tr>
        </table>
      </div>
    </td>

  </tr>
  </table>

  <!-- Amount -->
  <div style='background:#0e0e0e; border:1px solid #f97316;
              border-radius:10px; padding:14px 20px;
              text-align:center; margin-top:12px;'>
    <span style='color:#6b7280; font-size:13px;'>Total Amount</span><br>
    <span style='color:#f97316; font-size:24px;
                 font-weight:bold;'>PKR %24</span>
  </div>

  <!-- Passenger -->
  <div style='background:#0e0e0e; border:1px solid #1f2937;
              border-radius:10px; padding:12px 16px; margin-top:12px;'>
    <span style='color:#6b7280;'>Passenger: </span>
    <span style='color:#e5e7eb; font-weight:bold;'>%25</span>
    &nbsp;&nbsp;
    <span style='color:#6b7280;'>User ID: </span>
    <span style='color:#e5e7eb;'>%26</span>
  </div>

  <!-- Footer -->
  <div style='text-align:center; color:#374151;
              font-size:11px; margin-top:14px;'>
    ✈ Thank you for flying with AeroSync
  </div>

</div>)")
                                   .arg(statusBg)          // %1
                                   .arg(statusBorder)      // %2
                                   .arg(statusColor)       // %3
                                   .arg(statusIcon)        // %4
                                   .arg(statusText)        // %5
                                   .arg(QString::fromStdString(b->getFlightId()))  // %6
                                   .arg(from)              // %7
                                   .arg(to)                // %8
                                   .arg(dep)               // %9
                                   .arg(arr)               // %10
                                   .arg(seatNo)            // %11
                                   .arg(isBiz ? "#a78bfa" : "#fb923c")  // %12
                                   .arg(isBiz ? "Business" : "Economy") // %13
                                   .arg(bookingDate)       // %14
                                   .arg(bookingLabel)      // %15
                                   .arg(receiptId)         // %16
                                   .arg(QString::fromStdString(b->getBookingId()))  // %17
                                   .arg(txnId)             // %18
                                   .arg(bank)              // %19
                                   .arg(method)            // %20
                                   .arg(maskedAcc)         // %21
                                   .arg(payColor)          // %22
                                   .arg(payLabel)          // %23
                                   .arg(amount, 0, 'f', 2)          // %24
                                   .arg(QString::fromStdString(currentUser->getname()))    // %25
                                   .arg(QString::fromStdString(currentUser->get_userId())); // %26

                // ── Show in dialog ─────────────────────────────────────────
                QDialog dlg(this);
                dlg.setWindowTitle("Receipt — " +
                                   QString::fromStdString(b->getBookingId()));
                dlg.setMinimumSize(700, 580);
                dlg.setStyleSheet(
                    "QDialog{ background:#141414; }"
                    "QTextEdit{ background:#141414; border:none; }"
                    "QPushButton{ background:#f97316; color:#000000; border:none;"
                    "  border-radius:6px; padding:9px 22px; font-weight:bold; }"
                    "QPushButton:hover{ background:#fb923c; }"
                    "QPushButton:pressed{ background:#ea6c0a; }");

                auto* vd  = new QVBoxLayout(&dlg);
                auto* te  = new QTextEdit;
                te->setReadOnly(true);
                te->setHtml(html);
                vd->addWidget(te);

                auto* closeBtn = new QPushButton("Close");
                closeBtn->setCursor(Qt::PointingHandCursor);
                connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
                vd->addWidget(closeBtn, 0, Qt::AlignRight);

                dlg.exec();
            });

    // Right-click context menu for cancellation
    myBookingsTable->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(myBookingsTable, &QTableWidget::customContextMenuRequested,
            [this](const QPoint& pos) {
                int row = myBookingsTable->rowAt(pos.y());
                if (row < 0) return;

                // Retrieve the booking pointer stored in UserRole of col 0
                QTableWidgetItem* item = myBookingsTable->item(row, 0);
                if (!item) return;
                Booking* b = reinterpret_cast<Booking*>(
                    item->data(Qt::UserRole).value<quintptr>());
                if (!b) return;

                // Build menu — cancel only available under valid conditions
                QMenu menu(myBookingsTable);
                menu.setStyleSheet(
                    "QMenu{ background:#141414; color:#e5e7eb;"
                    "  border:1px solid #1f2937; border-radius:6px;"
                    "  font-family:'Segoe UI'; font-size:13px; padding:4px; }"
                    "QMenu::item{ padding:8px 20px; border-radius:4px; }"
                    "QMenu::item:selected{ background:#1a0d00; color:#f97316; }"
                    "QMenu::item:disabled{ color:#4b5563; }"
                    "QMenu::separator{ height:1px; background:#1f2937; margin:4px 8px; }");

                bool alreadyCancelled =
                    (b->getStatus() == Booking::BookingStatus::Cancelled);

                bool canCancel = false;
                if (!alreadyCancelled) {
                    Flight* f = ctx->flightRepo.getFlightById(b->getFlightId());
                    if (f && f->getStatus() == Flight::FlightStatus::Scheduled) {
                        // Check departure is more than 1 day away
                        QDateTime depDt = QDateTime::fromString(
                            QString::fromStdString(
                                datetime::toString(f->getDeparture())),
                            "dd-MM-yyyy HH:mm");
                        qint64 secsLeft = QDateTime::currentDateTime().secsTo(depDt);
                        canCancel = (secsLeft > 86400); // > 1 day in seconds
                    }
                }

                QAction* cancelAct = menu.addAction("🚫  Cancel Booking");
                cancelAct->setEnabled(canCancel);
                if (alreadyCancelled)
                    cancelAct->setText("❌  Already Cancelled");
                else if (!canCancel && !alreadyCancelled)
                    cancelAct->setText("🚫  Cancel Booking (not eligible)");

                QAction* chosen = menu.exec(
                    myBookingsTable->viewport()->mapToGlobal(pos));
                if (chosen == cancelAct && canCancel)
                    cancelBooking(b);
            });

    v->addWidget(myBookingsTable);

    auto* hint = new QLabel("💡 Double-click a booking to view its receipt  •  Right-click to cancel (if eligible)");
    hint->setObjectName("infoLabel");
    v->addWidget(hint);

    return w;
}

void UserDashboard::loadMyBookings() {
    myBookingsTable->setRowCount(0);

    auto history = ctx->bookingRepo.getUserHistory(currentUser->get_userId());

    for (auto* b : history) {
        int r = myBookingsTable->rowCount();
        myBookingsTable->insertRow(r);

        Flight* f  = ctx->flightRepo.getFlightById(b->getFlightId());
        QString from = f ? QString::fromStdString(f->getSource())      : "—";
        QString to   = f ? QString::fromStdString(f->getDestination()) : "—";

        QString status = (b->getStatus() == Booking::BookingStatus::Booked)
                             ? "✅ Booked" : "❌ Cancelled";

        auto* idItem = new QTableWidgetItem(QString::fromStdString(b->getBookingId()));
        // Store Booking* so the context menu can retrieve it without index lookup
        idItem->setData(Qt::UserRole,
                        QVariant::fromValue<quintptr>(reinterpret_cast<quintptr>(b)));
        myBookingsTable->setItem(r, 0, idItem);
        myBookingsTable->setItem(r, 1, new QTableWidgetItem(QString::fromStdString(b->getFlightId())));
        myBookingsTable->setItem(r, 2, new QTableWidgetItem(from));
        myBookingsTable->setItem(r, 3, new QTableWidgetItem(to));
        myBookingsTable->setItem(r, 4, new QTableWidgetItem(QString::fromStdString(b->getSeatNumber())));
        myBookingsTable->setItem(r, 5, new QTableWidgetItem(status));
    }
}

// ── Payment ────────────────────────────────────────────────────────────────
QWidget* UserDashboard::buildPaymentPage() {
    auto* w = new QWidget;
    auto* v = new QVBoxLayout(w);
    v->setContentsMargins(80, 30, 80, 30);
    v->setSpacing(12);

    auto* title = new QLabel("Payment");
    title->setObjectName("pageTitle");
    v->addWidget(title);

    paymentSummary = new QLabel(
        "No seat selected yet.\n"
        "Go to Search Flights → double-click a flight → pick a seat.");
    paymentSummary->setObjectName("infoLabel");
    paymentSummary->setTextFormat(Qt::RichText);
    v->addWidget(paymentSummary);

    v->addSpacing(10);

    v->addWidget(new QLabel("Select Bank:"));
    bankCombo = new QComboBox;
    bankCombo->addItems({
        "HBL – Habib Bank Limited",
        "UBL – United Bank Limited",
        "MCB – Muslim Commercial Bank",
        "ABL – Allied Bank Limited",
        "Meezan Bank",
        "Bank Alfalah",
        "Standard Chartered Pakistan",
        "Faysal Bank"
    });
    v->addWidget(bankCombo);

    v->addWidget(new QLabel("Payment Method:"));
    methodCombo = new QComboBox;
    methodCombo->addItems({"Card", "Cash", "Online Transfer", "Mobile Banking"});
    v->addWidget(methodCombo);

    v->addWidget(new QLabel("Account Number:"));
    accountNumber = new QLineEdit;
    accountNumber->setPlaceholderText("Enter your account / card number");
    accountNumber->setMaxLength(24);
    v->addWidget(accountNumber);

    v->addSpacing(16);
    auto* payBtn = new QPushButton("💳  Confirm & Pay");
    payBtn->setObjectName("actionBtn");
    connect(payBtn, &QPushButton::clicked, this, &UserDashboard::processPayment);
    v->addWidget(payBtn);
    v->addStretch();
    return w;
}

void UserDashboard::processPayment() {
    if (pendingFlightId.empty() || pendingSeatNo.empty()) {
        QMessageBox::warning(this, "No Seat Selected",
                             "Please search for a flight and select a seat first.");
        return;
    }
    if (accountNumber->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Missing Info",
                             "Please enter your account / card number.");
        return;
    }

    std::string bank   = bankCombo->currentText().toStdString();
    std::string method = methodCombo->currentText().toStdString();
    QString     accNo  = accountNumber->text().trimmed();

    QString maskedAcc = accNo.length() > 4
                            ? QString("*").repeated(accNo.length() - 4) + accNo.right(4)
                            : accNo;

    std::string txnId = "TXN" +
                        QDateTime::currentDateTime().toString("yyyyMMddHHmmss").toStdString();

    Payment payment(txnId, pendingPrice, bank, method,accNo.toStdString());


    Flight* f = ctx->flightRepo.getFlightById(pendingFlightId);
    if (!f) {
        QMessageBox::critical(this, "Error", "Flight not found.");
        return;
    }
    if (!f->bookSeat(pendingSeatNo)) {
        QMessageBox::critical(this, "Seat Unavailable",
                              "That seat was just taken. Please pick another.");
        pendingFlightId.clear(); pendingSeatNo.clear();
        return;
    }

    std::string bookingId = ctx->bookingRepo.getNextId();
    Booking booking(bookingId,
                    currentUser->get_userId(),
                    pendingFlightId,
                    pendingSeatNo,
                    datetime::now(),
                    Booking::BookingStatus::Booked);

    std::string receiptId = "REC-" + txnId;
    Receipt receipt(receiptId, bookingId, txnId);

    try {
        ctx->paymentRepo.addPayment(payment);  ctx->paymentRepo.save();
        ctx->bookingRepo.addBooking(booking);  ctx->bookingRepo.save();
        ctx->flightRepo.save();
        ctx->receiptRepo.addReceipt(receipt);  ctx->receiptRepo.save();
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Save Error", QString::fromStdString(e.what()));
        return;
    }

    QString dep = QString::fromStdString(datetime::toString(f->getDeparture()));
    QString arr = QString::fromStdString(datetime::toString(f->getArrival()));
    bool isBiz  = !pendingSeatNo.empty() && pendingSeatNo[0] == 'B';

    receiptDisplay->setHtml(QString(R"(
<div style='font-family:Segoe UI,sans-serif; padding:10px; background:#141414;'>

  <!-- Header -->
  <div style='background:#0e0e0e; border:1px solid #1f2937;
              border-radius:12px; padding:20px;
              text-align:center; margin-bottom:16px;'>
    <div style='color:#f97316; font-size:24px;
                font-weight:bold; margin:6px 0;'>✈ AeroSync</div>
    <div style='color:#6b7280; font-size:11px;
                letter-spacing:3px;'>BOOKING CONFIRMATION</div>
  </div>

  <!-- Status badge -->
  <div style='background:#052e16; border:1px solid #166534;
              border-radius:8px; padding:10px 16px;
              text-align:center; margin-bottom:16px;'>
    <span style='color:#4ade80; font-size:14px; font-weight:bold;'>
      ✅ Payment Successful — Booking Confirmed
    </span>
  </div>

  <!-- Two column cards -->
  <table width='100%%' cellspacing='8' cellpadding='0'>
  <tr>

    <!-- Flight Info -->
    <td width='50%%' valign='top'>
      <div style='background:#0e0e0e; border:1px solid #1f2937;
                  border-radius:10px; padding:14px;'>
        <div style='color:#f97316; font-weight:bold;
                    font-size:13px; margin-bottom:10px;
                    border-bottom:1px solid #1f2937; padding-bottom:6px;'>
          ✈ Flight Details
        </div>
        <table cellpadding='5' width='100%%'>
          <tr><td style='color:#6b7280;'>Flight ID</td>
              <td style='color:#e5e7eb; font-weight:bold;'>%1</td></tr>
          <tr><td style='color:#6b7280;'>Route</td>
              <td style='color:#e5e7eb;'>%2 to %3</td></tr>
          <tr><td style='color:#6b7280;'>Departure</td>
              <td style='color:#e5e7eb;'>%4</td></tr>
          <tr><td style='color:#6b7280;'>Arrival</td>
              <td style='color:#e5e7eb;'>%5</td></tr>
          <tr><td style='color:#6b7280;'>Seat</td>
              <td style='color:#e5e7eb;'>%6</td></tr>
          <tr><td style='color:#6b7280;'>Class</td>
              <td style='color:%7; font-weight:bold;'>%8</td></tr>
        </table>
      </div>
    </td>

    <!-- Payment Info -->
    <td width='50%%' valign='top'>
      <div style='background:#0e0e0e; border:1px solid #1f2937;
                  border-radius:10px; padding:14px;'>
        <div style='color:#f97316; font-weight:bold;
                    font-size:13px; margin-bottom:10px;
                    border-bottom:1px solid #1f2937; padding-bottom:6px;'>
          💳 Payment Details
        </div>
        <table cellpadding='5' width='100%%'>
          <tr><td style='color:#6b7280;'>Receipt ID</td>
              <td style='color:#e5e7eb;'>%9</td></tr>
          <tr><td style='color:#6b7280;'>Booking ID</td>
              <td style='color:#e5e7eb;'>%10</td></tr>
          <tr><td style='color:#6b7280;'>Transaction</td>
              <td style='color:#e5e7eb;'>%11</td></tr>
          <tr><td style='color:#6b7280;'>Bank</td>
              <td style='color:#e5e7eb;'>%12</td></tr>
          <tr><td style='color:#6b7280;'>Method</td>
              <td style='color:#e5e7eb;'>%13</td></tr>
          <tr><td style='color:#6b7280;'>Account</td>
              <td style='color:#e5e7eb;'>%14</td></tr>
        </table>
      </div>
    </td>

  </tr>
  </table>

  <!-- Amount -->
  <div style='background:#0e0e0e; border:1px solid #f97316;
              border-radius:10px; padding:14px 20px;
              text-align:center; margin-top:12px;'>
    <span style='color:#6b7280; font-size:13px;'>Total Amount Paid</span><br>
    <span style='color:#f97316; font-size:24px;
                 font-weight:bold;'>PKR %15</span>
  </div>

  <!-- Passenger -->
  <div style='background:#0e0e0e; border:1px solid #1f2937;
              border-radius:10px; padding:12px 16px; margin-top:12px;'>
    <span style='color:#6b7280;'>Passenger: </span>
    <span style='color:#e5e7eb; font-weight:bold;'>%16</span>
    &nbsp;&nbsp;
    <span style='color:#6b7280;'>User ID: </span>
    <span style='color:#e5e7eb;'>%17</span>
  </div>

  <!-- Footer -->
  <div style='text-align:center; color:#374151;
              font-size:11px; margin-top:14px;'>
    ✈ Thank you for flying with AeroSync
  </div>

</div>
)")
                                .arg(QString::fromStdString(pendingFlightId))
                                .arg(QString::fromStdString(f->getSource()))
                                .arg(QString::fromStdString(f->getDestination()))
                                .arg(dep)
                                .arg(arr)
                                .arg(QString::fromStdString(pendingSeatNo))
                                .arg(isBiz ? "#a78bfa" : "#fb923c")
                                .arg(isBiz ? "Business" : "Economy")
                                .arg(QString::fromStdString(receiptId))
                                .arg(QString::fromStdString(bookingId))
                                .arg(QString::fromStdString(txnId))
                                .arg(QString::fromStdString(bank))
                                .arg(QString::fromStdString(method))
                                .arg(maskedAcc)
                                .arg(pendingPrice, 0, 'f', 2)
                                .arg(QString::fromStdString(currentUser->getname()))
                                .arg(QString::fromStdString(currentUser->get_userId()))
                            );

    pendingFlightId.clear();
    pendingSeatNo.clear();
    pendingPrice = 0.0;
    accountNumber->clear();

    QMessageBox::information(this, "Payment Successful",
                             "Your booking is confirmed!\nNavigating to your receipt...");
    sidebar->setCurrentRow(3);
    contentStack->setCurrentIndex(3);
}

// ── Receipt ────────────────────────────────────────────────────────────────
QWidget* UserDashboard::buildReceiptPage() {
    auto* w = new QWidget;
    auto* v = new QVBoxLayout(w);
    v->setContentsMargins(20, 20, 20, 20);
    v->setSpacing(10);

    auto* title = new QLabel("Receipt");
    title->setObjectName("pageTitle");
    v->addWidget(title);

    receiptDisplay = new QTextEdit;
    receiptDisplay->setReadOnly(true);
    receiptDisplay->setObjectName("receipt");
    receiptDisplay->setPlaceholderText("Complete a payment to see your receipt here.");
    v->addWidget(receiptDisplay);
    return w;
}

void UserDashboard::cancelBooking(Booking* b)
{
    // ── Confirm ────────────────────────────────────────────────────────────
    auto reply = QMessageBox::question(
        this, "Confirm Cancellation",
        QString("Are you sure you want to cancel booking <b>%1</b>?<br>"
                "Your payment will be marked as <b>Refunded</b>.")
            .arg(QString::fromStdString(b->getBookingId())),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (reply != QMessageBox::Yes) return;

    // ── 1. Release the seat on the flight ──────────────────────────────────
    Flight* f = ctx->flightRepo.getFlightById(b->getFlightId());
    if (!f) {
        QMessageBox::critical(this, "Error",
                              "Flight data lost or corrupted. Cancellation aborted.");
        return;
    }
    f->releaseSeat(b->getSeatNumber());
    ctx->flightRepo.save();

    // ── 2. Find receipt → get payment ID ──────────────────────────────────
    Receipt* receipt = ctx->receiptRepo.searchByBookingId(b->getBookingId());
    if (!receipt) {
        QMessageBox::critical(this, "Data Error",
                              "Receipt data lost or corrupted. Cancellation aborted.\n"
                              "Please contact support.");
        // Roll back seat release
        f->bookSeat(b->getSeatNumber());
        ctx->flightRepo.save();
        return;
    }

    std::string paymentId = receipt->getPaymentId();

    // ── 3. Find payment → mark Refunded ───────────────────────────────────
    Payment* payment = ctx->paymentRepo.getPaymentById(paymentId);
    if (!payment) {
        QMessageBox::critical(this, "Data Error",
                              "Payment record lost or corrupted. Cancellation aborted.\n"
                              "Please contact support.");
        // Roll back seat release
        f->bookSeat(b->getSeatNumber());
        ctx->flightRepo.save();
        return;
    }
    payment->updatePaymentStatus(Payment::PaymentStatus::Refunded);
    ctx->paymentRepo.save();

    // ── 4. Cancel the booking ──────────────────────────────────────────────
    b->cancel();
    ctx->bookingRepo.save();

    // ── 5. Reload the bookings table ───────────────────────────────────────
    loadMyBookings();

    QMessageBox::information(this, "Booking Cancelled",
                             "✅ Booking cancelled successfully.\n"
                             "Your refund has been initiated.");
}

void UserDashboard::logout() {
    close();
    (new LoginWindow(ctx))->show();
}

void UserDashboard::applyStyles() {
    setStyleSheet(R"(
        QMainWindow, QWidget {
            background:#0e0e0e; color:#e5e7eb;
            font-family:'Segoe UI'; font-size:13px;
        }
        QListWidget#sidebar {
            background:#0a0a0a; border:none;
            color:#6b7280; font-size:14px;
        }
        QListWidget#sidebar::item {
            padding:18px 16px; border-bottom:1px solid #1f2937;
        }
        QListWidget#sidebar::item:selected {
            background:#1a0d00; color:#f97316;
            border-left:4px solid #f97316;
        }
        QListWidget#sidebar::item:hover { background:#141414; }
        QLabel#pageTitle {
            font-size:20px; font-weight:bold;
            color:#f97316; margin-bottom:4px;
        }
        QLabel#infoLabel {
            background:#141414; border:1px solid #1f2937;
            border-radius:8px; padding:12px; color:#6b7280;
        }
        QCheckBox {
            color:#6b7280; font-size:13px;
        }
        QCheckBox::indicator {
            width:16px; height:16px;
            border:1px solid #374151; border-radius:3px;
            background:#141414;
        }
        QCheckBox::indicator:checked {
            background:#f97316; border:1px solid #f97316;
        }
        QTableWidget {
            background:#0e0e0e; border:none;
            gridline-color:#1f2937; color:#e5e7eb;
            alternate-background-color:#111111;
        }
        QHeaderView::section {
            background:#141414; color:#f97316;
            padding:8px; border:none;
            border-bottom:2px solid #f97316; font-weight:bold;
        }
        QHeaderView::section:hover { background:#1a1a1a; color:#fb923c; }
        QTableWidget::item { padding:4px 6px; }
        QTableWidget::item:hover { background:#1a1a1a; color:#f9fafb; }
        QTableWidget::item:selected {
            background:#1a0d00; color:#ffffff;
            border-left:3px solid #f97316;
        }
        QLineEdit, QComboBox, QDateEdit {
            background:#141414; border:1px solid #1f2937;
            border-radius:6px; padding:8px 12px; color:#f9fafb;
        }
        QLineEdit:focus, QComboBox:focus, QDateEdit:focus {
            border:1px solid #f97316;
        }
        QDateEdit::drop-down {
            background:#1f2937; border:none;
            border-radius:0 6px 6px 0; width:28px;
        }
        QDateEdit::drop-down:hover { background:#374151; }
        QCalendarWidget QAbstractItemView {
            background:#141414; color:#e5e7eb;
            selection-background-color:#f97316;
            selection-color:#000000;
        }
        QCalendarWidget QWidget#qt_calendar_navigationbar {
            background:#0e0e0e;
        }
        QCalendarWidget QToolButton {
            color:#f97316; background:transparent; font-weight:bold;
        }
        QCalendarWidget QToolButton:hover { color:#fb923c; }
        QPushButton#actionBtn {
            background:#f97316; color:#000000; border:none;
            border-radius:6px; padding:9px 22px; font-weight:bold;
        }
        QPushButton#actionBtn:hover { background:#fb923c; }
        QPushButton#actionBtn:pressed { background:#ea6c0a; }
        QTextEdit#receipt {
            background:#141414; border:1px solid #1f2937;
            border-radius:8px; padding:12px;
            font-family:'Segoe UI'; font-size:13px;
            color:#f97316; line-height:1.5;
        }
    )");
}