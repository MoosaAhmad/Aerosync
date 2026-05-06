#include "AdminDashboard.h"
#include "LoginWindow.h"
#include "Flight.h"
#include "Seat.h"
#include "Receipt.h"
#include "Payment.h"
#include "ProfitCalculator.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QMessageBox>
#include <QScrollArea>
#include <QDateEdit>
#include <QTimeEdit>
#include <QTextEdit>
#include <QEvent>
#include <algorithm>
#include <cmath>

AdminDashboard::AdminDashboard(Admin* curAdmin, AppContext* ctx, QWidget* parent)
    : QMainWindow(parent), ctx(ctx), currentAdmin(curAdmin)
{
    setWindowTitle("AeroSync – Admin Dashboard");
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
        "  ✈   Manage Flights",
        "  📋   All Bookings",
        "  👥   All Users",
        "  💰   Profit Calculator",
        "  🔒   Change Password",
        "  🚪   Logout"
    });
    sidebar->setCurrentRow(0);
    connect(sidebar, &QListWidget::currentRowChanged,
            this, &AdminDashboard::onSidebarClicked);

    contentStack = new QStackedWidget;
    contentStack->addWidget(buildFlightsPage());  // 0
    contentStack->addWidget(buildBookingsPage()); // 1
    contentStack->addWidget(buildUsersPage());    // 2
    contentStack->addWidget(buildProfitPage());   // 3

    mainLay->addWidget(sidebar);
    mainLay->addWidget(contentStack, 1);

    loadFlightsTable();
    applyStyles();
}

void AdminDashboard::onSidebarClicked(int row) {
    if (row == 5) { logout(); return; }
    if (row == 4) { openChangePassword(); return; }
    if (row == 0) loadFlightsTable();
    if (row == 1) loadAllBookings();
    if (row == 2) loadAllUsers();
    contentStack->setCurrentIndex(row);
}

void AdminDashboard::openChangePassword()
{
    int prevPage = contentStack->currentIndex();
    ChangePasswordDialog dlg(currentAdmin, ctx, this);
    if (dlg.exec() == QDialog::Accepted) {
        QMessageBox::information(this, "Password Updated",
                                 "✅  Your password has been changed successfully.");
    }
    sidebar->setCurrentRow(prevPage);
    ctx->userRepo.saveAdmins();
    contentStack->setCurrentIndex(prevPage);
}

// ── Manage Flights ─────────────────────────────────────────────────────────
QWidget* AdminDashboard::buildFlightsPage() {
    auto* w = new QWidget;
    auto* v = new QVBoxLayout(w);
    v->setContentsMargins(20, 20, 20, 20);
    v->setSpacing(10);

    auto* title = new QLabel("Manage Flights");
    title->setObjectName("pageTitle");
    v->addWidget(title);

    // Hint label for double-click
    auto* hint = new QLabel("💡 Double-click a flight row to view its seat map");
    hint->setObjectName("infoLabel");
    v->addWidget(hint);

    flightsTable = new QTableWidget(0, 7);
    flightsTable->setHorizontalHeaderLabels(
        { "Flight ID","From","To","Departure","Arrival","Status","Avail. Seats" });
    flightsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    flightsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    flightsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    flightsTable->setAlternatingRowColors(true);

    // ── double-click → show seat map ──────────────────────────────────────
    connect(flightsTable, &QTableWidget::cellDoubleClicked,
            this, &AdminDashboard::onFlightDoubleClicked);

    v->addWidget(flightsTable, 2);

    // Status update row
    auto* statusRow = new QHBoxLayout;
    fStatus = new QComboBox;
    fStatus->addItems({ "Scheduled","Delayed","Cancelled","Departed","Arrived" });
    auto* updateBtn = new QPushButton("Update Status of Selected");
    updateBtn->setObjectName("actionBtn");
    updateBtn->setCursor(Qt::PointingHandCursor);
    connect(updateBtn, &QPushButton::clicked, this, &AdminDashboard::updateFlightStatus);
    statusRow->addWidget(new QLabel("New Status:"));
    statusRow->addWidget(fStatus);
    statusRow->addWidget(updateBtn);
    statusRow->addStretch();
    v->addLayout(statusRow);

    // Add Flight form
    auto* addBox = new QGroupBox("Add New Flight");
    addBox->setObjectName("grp");
    auto* grid = new QGridLayout(addBox);
    grid->setHorizontalSpacing(10);
    grid->setVerticalSpacing(8);

    fId = new QLineEdit; fId->setReadOnly(true);
    fFrom = new QLineEdit; fFrom->setPlaceholderText("e.g. Lahore");
    fTo   = new QLineEdit; fTo->setPlaceholderText("e.g. Dubai");

    fDepDate = new QDateEdit(QDate::currentDate());
    fDepDate->setDisplayFormat("dd-MM-yyyy");
    fDepDate->setCalendarPopup(true);
    fDepDate->setMinimumDate(QDate::currentDate());

    fArrDate = new QDateEdit(QDate::currentDate());
    fArrDate->setDisplayFormat("dd-MM-yyyy");
    fArrDate->setCalendarPopup(true);
    fArrDate->setMinimumDate(QDate::currentDate());

    fDepTime = new QTimeEdit(QTime(0, 0));
    fDepTime->setDisplayFormat("HH:mm");

    fArrTime = new QTimeEdit(QTime(1, 0));
    fArrTime->setDisplayFormat("HH:mm");

    fEcoSeats = new QSpinBox;
    fEcoSeats->setRange(0, 500);
    fEcoSeats->setSpecialValueText("0");
    fEcoSeats->setSuffix(" seats");

    fBizSeats = new QSpinBox;
    fBizSeats->setRange(0, 200);
    fBizSeats->setSpecialValueText("0");
    fBizSeats->setSuffix(" seats");

    fEcoPrice = new QDoubleSpinBox;
    fEcoPrice->setRange(0, 9'999'999);
    fEcoPrice->setDecimals(0);
    fEcoPrice->setPrefix("PKR ");
    fEcoPrice->setSingleStep(1000);

    fBizPrice = new QDoubleSpinBox;
    fBizPrice->setRange(0, 9'999'999);
    fBizPrice->setDecimals(0);
    fBizPrice->setPrefix("PKR ");
    fBizPrice->setSingleStep(1000);

    fId->setText(QString::fromStdString(ctx->flightRepo.getNextId()));
    // Calendar emoji on date popup buttons
    if (auto* btn = fDepDate->findChild<QAbstractButton*>()) btn->setText("v");
    if (auto* btn = fArrDate->findChild<QAbstractButton*>()) btn->setText("v");

    // Up/down arrows on time spinners
    // (handled via QSS ::up-arrow / ::down-arrow in applyStyles)

    grid->addWidget(new QLabel("Flight ID"),   0, 0); grid->addWidget(fId,       0, 1);
    grid->addWidget(new QLabel("From"),        0, 2); grid->addWidget(fFrom,     0, 3);
    grid->addWidget(new QLabel("To"),          0, 4); grid->addWidget(fTo,       0, 5);
    grid->addWidget(new QLabel("Depart Date"), 1, 0); grid->addWidget(fDepDate,  1, 1);
    grid->addWidget(new QLabel("Depart Time"), 1, 2); grid->addWidget(fDepTime,  1, 3);
    grid->addWidget(new QLabel("Arrive Date"), 1, 4); grid->addWidget(fArrDate,  1, 5);
    grid->addWidget(new QLabel("Arrive Time"), 2, 0); grid->addWidget(fArrTime,  2, 1);
    grid->addWidget(new QLabel("Eco. Seats"),  2, 2); grid->addWidget(fEcoSeats, 2, 3);
    grid->addWidget(new QLabel("Biz. Seats"),  2, 4); grid->addWidget(fBizSeats, 2, 5);
    grid->addWidget(new QLabel("Eco. Price"),  3, 0); grid->addWidget(fEcoPrice, 3, 1);
    grid->addWidget(new QLabel("Biz. Price"),  3, 2); grid->addWidget(fBizPrice, 3, 3);

    v->addWidget(addBox);

    auto* btnRow = new QHBoxLayout;
    auto* addBtn = new QPushButton("➕  Add Flight");
    addBtn->setObjectName("actionBtn");
    addBtn->setCursor(Qt::PointingHandCursor);
    auto* delBtn = new QPushButton("🗑  Delete Selected");
    delBtn->setObjectName("dangerBtn");
    delBtn->setCursor(Qt::PointingHandCursor);
    connect(addBtn, &QPushButton::clicked, this, &AdminDashboard::addFlight);
    connect(delBtn, &QPushButton::clicked, this, &AdminDashboard::deleteFlight);
    btnRow->addWidget(addBtn); btnRow->addWidget(delBtn); btnRow->addStretch();
    v->addLayout(btnRow);

    return w;
}

// ── Seat Map Dialog ────────────────────────────────────────────────────────
void AdminDashboard::onFlightDoubleClicked(int row, int /*col*/)
{
    QString flightId = flightsTable->item(row, 0)->text();
    Flight* f = ctx->flightRepo.getFlightById(flightId.toStdString());
    if (!f) return;

    showSeatMap(f);
}

void AdminDashboard::showSeatMap(Flight* flight)
{
    auto availSeats    = flight->getAvailableSeats();
    auto reservedSeats = flight->getReservedSeats();

    if (availSeats.empty() && reservedSeats.empty()) {
        QMessageBox::information(this, "No Seats",
                                 "This flight has no seat data.");
        return;
    }

    QDialog dlg(this);
    dlg.setWindowTitle("Seat Map  –  " + QString::fromStdString(flight->getId()));
    dlg.setMinimumSize(580, 480);
    dlg.setStyleSheet(
        "QDialog{"
        "  background:#0d0a06; color:#d4c5a9;"
        "  font-family:'Garamond','Times New Roman',serif; font-size:13px;"
        "}"
        "QScrollArea{ border:none; background:#110e08; }"
        "QWidget#scrollContent{ background:#110e08; }");

    auto* vMain = new QVBoxLayout(&dlg);
    vMain->setSpacing(10);

    // ── Header ────────────────────────────────────────────────────────────
    auto* info = new QLabel(QString(
                                "<b style='color:#f97316; font-size:15px;'>%1 &nbsp;→&nbsp; %2</b>"
                                "<span style='color:#6b5a40;'> &nbsp;|&nbsp; %3</span>")
                                .arg(QString::fromStdString(flight->getSource()))
                                .arg(QString::fromStdString(flight->getDestination()))
                                .arg(QString::fromStdString(datetime::toString(flight->getDeparture()))));
    info->setTextFormat(Qt::RichText);
    vMain->addWidget(info);

    // ── Stats bar ─────────────────────────────────────────────────────────
    int totalSeats  = (int)(availSeats.size() + reservedSeats.size());
    int bookedCount = (int)reservedSeats.size();
    int availCount  = (int)availSeats.size();

    auto* stats = new QLabel(QString(
                                 "<span style='color:#6b5a40;'>Total: </span>"
                                 "<b style='color:#e8d5b0;'>%1</b>"
                                 "&nbsp;&nbsp;&nbsp;"
                                 "<span style='color:#6b5a40;'>Available: </span>"
                                 "<b style='color:#fb923c;'>%2</b>"
                                 "&nbsp;&nbsp;&nbsp;"
                                 "<span style='color:#6b5a40;'>Booked: </span>"
                                 "<b style='color:#fb923c;'>%3</b>")
                                 .arg(totalSeats).arg(availCount).arg(bookedCount));
    stats->setTextFormat(Qt::RichText);
    vMain->addWidget(stats);

    // ── Legend ────────────────────────────────────────────────────────────
    auto* legend = new QLabel(
        "<span style='color:#fb923c;'>▣ Economy – Available</span>"
        "&nbsp;&nbsp;&nbsp;"
        "<span style='color:#a78bfa;'>▣ Business – Available</span>"
        "&nbsp;&nbsp;&nbsp;"
        "<span style='color:#4a3520;'>▣ Booked — click to view details</span>");
    legend->setTextFormat(Qt::RichText);
    legend->setWordWrap(true);
    vMain->addWidget(legend);

    // ── Seat grid ─────────────────────────────────────────────────────────
    auto* scrollContent = new QWidget;
    scrollContent->setObjectName("scrollContent");
    auto* grid = new QGridLayout(scrollContent);
    grid->setSpacing(6);

    std::vector<Seat*> ecoAvail, ecoBkd, bizAvail, bizBkd;
    for (auto* s : availSeats) {
        if (s->getSeatClass() == Seat::SeatClass::Economy) ecoAvail.push_back(s);
        else                                               bizAvail.push_back(s);
    }
    for (auto* s : reservedSeats) {
        if (s->getSeatClass() == Seat::SeatClass::Economy) ecoBkd.push_back(s);
        else                                               bizBkd.push_back(s);
    }

    int currentRow = 0;
    const int COLS = 5;

    auto addSection = [&](const QString& label,
                          const std::vector<Seat*>& seats,
                          bool isBooked,
                          bool isBiz)
    {
        if (seats.empty()) return;

        auto* hdr = new QLabel(label);
        hdr->setStyleSheet(isBiz
                               ? "color:#a78bfa; font-weight:bold; font-size:12px; margin-top:6px;"
                               : "color:#fb923c; font-weight:bold; font-size:12px; margin-top:6px;");
        grid->addWidget(hdr, currentRow, 0, 1, COLS);
        currentRow++;

        int col = 0;
        for (auto* s : seats) {
            std::string seatNo = s->getSeatNumber();
            double      price  = s->getPrice();

            auto* btn = new QPushButton(
                QString::fromStdString(seatNo) + "\n" +
                QString::number(price / 1000.0, 'f', 0) + "k");
            btn->setFixedSize(90, 58);
            btn->setToolTip(QString("%1  |  PKR %2  |  %3")
                                .arg(QString::fromStdString(seatNo))
                                .arg(price, 0, 'f', 0)
                                .arg(isBooked ? "Booked – click for details" : "Available"));

            if (isBooked) {
                // ── Booked: dim base, WITH hover glow, pointer cursor, clickable ──
                btn->setCursor(Qt::PointingHandCursor);
                btn->setStyleSheet(isBiz
                                       ? "QPushButton{"
                                         "  background:#0d0a20; color:#3a2f5a;"
                                         "  border:1px solid #1e1a38;"
                                         "  border-radius:6px; font-size:11px; font-weight:bold;"
                                         "}"
                                         "QPushButton:hover{"
                                         "  background:#1a1540; color:#7060a0;"
                                         "  border:2px solid #4a3a80;"
                                         "}"
                                         "QPushButton:pressed{ background:#0a0818; }"
                                       : "QPushButton{"
                                         "  background:#1a1008; color:#4a3520;"
                                         "  border:1px solid #2a1e10;"
                                         "  border-radius:6px; font-size:11px; font-weight:bold;"
                                         "}"
                                         "QPushButton:hover{"
                                         "  background:#2e1c0a; color:#8a6040;"
                                         "  border:2px solid #5a3a18;"
                                         "}"
                                         "QPushButton:pressed{ background:#120b04; }");

                // Click → look up booking and show info dialog
                connect(btn, &QPushButton::clicked,
                        [=, &dlg]()
                        {
                            Booking* b = ctx->bookingRepo.findBookedSeat(
                                flight->getId(), seatNo);

                            if (!b) {
                                QMessageBox::critical(&dlg, "Data Error",
                                                      "⚠️  No booking record found for seat "
                                                          + QString::fromStdString(seatNo) +
                                                          ".\nData may be corrupted or out of sync.");
                                return;
                            }

                            // ── Booking detail popup ──────────────────────
                            QDialog info(&dlg);
                            info.setWindowTitle("Booking Details – "
                                                + QString::fromStdString(seatNo));
                            info.setMinimumWidth(400);
                            info.setStyleSheet(
                                "QDialog{ background:#0d0a06; color:#d4c5a9;"
                                "font-family:'Garamond','Times New Roman',serif;"
                                "font-size:13px; }"
                                "QLabel{ padding:2px; }");

                            auto* vInfo = new QVBoxLayout(&info);
                            vInfo->setSpacing(10);

                            auto* ttl = new QLabel(
                                "<b style='color:#f97316; font-size:14px;'>"
                                "🎫 &nbsp; Booking Information</b>");
                            ttl->setTextFormat(Qt::RichText);
                            vInfo->addWidget(ttl);

                            // Card: all fields except User ID (rendered separately as clickable)
                            auto* card = new QLabel(QString(
                                                        "<div style='background:#110e08;"
                                                        "border:1px solid #2a1e0a;"
                                                        "border-radius:10px; padding:14px;'>"
                                                        "<table cellpadding='5' width='100%'>"
                                                        "<tr>"
                                                        "  <td style='color:#6b5a40;'>Booking ID</td>"
                                                        "  <td style='color:#e8d5b0; font-weight:bold;'>%1</td>"
                                                        "</tr><tr>"
                                                        "  <td style='color:#6b5a40;'>Flight</td>"
                                                        "  <td style='color:#e8d5b0;'>%2</td>"
                                                        "</tr><tr>"
                                                        "  <td style='color:#6b5a40;'>Seat</td>"
                                                        "  <td style='color:#e8d5b0;'>%3</td>"
                                                        "</tr><tr>"
                                                        "  <td style='color:#6b5a40;'>Booked On</td>"
                                                        "  <td style='color:#e8d5b0;'>%4</td>"
                                                        "</tr><tr>"
                                                        "  <td style='color:#6b5a40;'>Status</td>"
                                                        "  <td style='color:#fb923c; font-weight:bold;'>%5</td>"
                                                        "</tr>"
                                                        "</table></div>")
                                                        .arg(QString::fromStdString(b->getBookingId()))
                                                        .arg(QString::fromStdString(b->getFlightId()))
                                                        .arg(QString::fromStdString(b->getSeatNumber()))
                                                        .arg(QString::fromStdString(
                                                            datetime::toString(b->getBookingDate())))
                                                        .arg(QString::fromStdString(b->getStatusString())));
                            card->setTextFormat(Qt::RichText);
                            vInfo->addWidget(card);

                            // ── User ID row — double-clickable link ───────
                            QString bUserId = QString::fromStdString(b->getUserId());
                            auto* userRow = new QWidget;
                            userRow->setStyleSheet(
                                "QWidget{ background:#110e08; border:1px solid #2a1e0a;"
                                "border-radius:10px; padding:4px; }");
                            auto* userRowLay = new QHBoxLayout(userRow);
                            userRowLay->setContentsMargins(14, 6, 14, 6);

                            auto* userLbl = new QLabel(
                                "<span style='color:#6b5a40;'>User ID</span>");
                            userLbl->setTextFormat(Qt::RichText);

                            auto* userBtn = new QPushButton(bUserId);
                            userBtn->setCursor(Qt::PointingHandCursor);
                            userBtn->setToolTip("Double-click to view this user's profile & bookings");
                            userBtn->setStyleSheet(
                                "QPushButton{ background:transparent; color:#f97316;"
                                "border:none; font-size:13px; font-weight:bold;"
                                "text-decoration:underline; padding:0; }"
                                "QPushButton:hover{ color:#fb923c; }"
                                "QPushButton:pressed{ color:#ea6c0a; }");

                            // Use event filter to detect double-click on the button
                            class DblClickFilter : public QObject {
                            public:
                                QPushButton*    btn;
                                QDialog*        infoDlg;
                                QDialog*        seatDlg;
                                AdminDashboard* dash;
                                QString         uid;
                                DblClickFilter(QPushButton* b, QDialog* id,
                                               QDialog* sd, AdminDashboard* d,
                                               const QString& u)
                                    : QObject(b), btn(b), infoDlg(id),
                                    seatDlg(sd), dash(d), uid(u) {}
                                bool eventFilter(QObject* obj, QEvent* ev) override {
                                    if (obj == btn &&
                                        ev->type() == QEvent::MouseButtonDblClick) {
                                        QString uName = uid;
                                        for (auto* u : dash->ctx->userRepo.getUsers()) {
                                            if (QString::fromStdString(u->get_userId()) == uid) {
                                                uName = QString::fromStdString(u->getname());
                                                break;
                                            }
                                        }
                                        infoDlg->accept();
                                        seatDlg->accept();
                                        dash->sidebar->setCurrentRow(2);
                                        dash->loadAllUsers();
                                        dash->contentStack->setCurrentIndex(2);
                                        for (int r = 0; r < dash->usersTable->rowCount(); ++r) {
                                            if (dash->usersTable->item(r, 0) &&
                                                dash->usersTable->item(r, 0)->text() == uid) {
                                                dash->usersTable->selectRow(r);
                                                dash->usersTable->scrollToItem(
                                                    dash->usersTable->item(r, 0));
                                                break;
                                            }
                                        }
                                        dash->showUserBookings(uid, uName);
                                        return true;
                                    }
                                    return false;
                                }
                            };
                            userBtn->installEventFilter(
                                new DblClickFilter(userBtn, &info, &dlg, this, bUserId));

                            auto* hintLbl = new QLabel(
                                "<span style='color:#4a3520; font-size:11px;'>"
                                "  (double-click to open profile)</span>");
                            hintLbl->setTextFormat(Qt::RichText);

                            userRowLay->addWidget(userLbl);
                            userRowLay->addWidget(userBtn);
                            userRowLay->addWidget(hintLbl);
                            userRowLay->addStretch();
                            vInfo->addWidget(userRow);

                            auto* closeInfo = new QPushButton("Close");
                            closeInfo->setCursor(Qt::PointingHandCursor);
                            closeInfo->setStyleSheet(
                                "QPushButton{ background:#1a1208; color:#8a7560;"
                                "border:1px solid #3a2a10; border-radius:6px;"
                                "padding:8px 20px; }"
                                "QPushButton:hover{ background:#241a0a; color:#b09070;"
                                "border:1px solid #6a4a20; }");
                            connect(closeInfo, &QPushButton::clicked,
                                    &info, &QDialog::accept);
                            vInfo->addWidget(closeInfo, 0, Qt::AlignRight);

                            info.exec();
                        });

            } else {
                // ── Available: bright color, NO hover, arrow cursor (read-only) ──
                btn->setCursor(Qt::ArrowCursor);
                btn->setStyleSheet(isBiz
                                       ? "QPushButton{"
                                         "  background:#1a1a3a; color:#a78bfa;"
                                         "  border:1px solid #4c3a9e;"
                                         "  border-radius:6px; font-size:11px; font-weight:bold;"
                                         "}"
                                       : "QPushButton{"
                                         "  background:#221a0e; color:#fb923c;"
                                         "  border:1px solid #7c3a0e;"
                                         "  border-radius:6px; font-size:11px; font-weight:bold;"
                                         "}");
                // No connect — available seats are just informational for admin
            }

            grid->addWidget(btn, currentRow + col / COLS, col % COLS);
            col++;
        }
        currentRow += (col + COLS - 1) / COLS;
        currentRow++;   // blank gap between sections
    };

    addSection("Economy – Available",  ecoAvail, false, false);
    addSection("Economy – Booked",     ecoBkd,   true,  false);
    addSection("Business – Available", bizAvail, false, true);
    addSection("Business – Booked",    bizBkd,   true,  true);

    auto* scroll = new QScrollArea;
    scroll->setWidget(scrollContent);
    scroll->setWidgetResizable(true);
    vMain->addWidget(scroll);

    // ── Close button ──────────────────────────────────────────────────────
    auto* closeBtn = new QPushButton("Close");
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(
        "QPushButton{"
        "  background:#1a1208; color:#8a7560;"
        "  border:1px solid #3a2a10; border-radius:6px; padding:8px 20px;"
        "}"
        "QPushButton:hover{"
        "  background:#241a0a; color:#b09070; border:1px solid #6a4a20;"
        "}");
    connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    vMain->addWidget(closeBtn, 0, Qt::AlignRight);

    dlg.exec();
}

// ── loadFlightsTable ───────────────────────────────────────────────────────
void AdminDashboard::loadFlightsTable() {
    flightsTable->setRowCount(0);

    for (auto* f : ctx->flightRepo.share()) {
        int r = flightsTable->rowCount();
        flightsTable->insertRow(r);

        flightsTable->setItem(r, 0, new QTableWidgetItem(QString::fromStdString(f->getId())));
        flightsTable->setItem(r, 1, new QTableWidgetItem(QString::fromStdString(f->getSource())));
        flightsTable->setItem(r, 2, new QTableWidgetItem(QString::fromStdString(f->getDestination())));
        flightsTable->setItem(r, 3, new QTableWidgetItem(QString::fromStdString(datetime::toString(f->getDeparture()))));
        flightsTable->setItem(r, 4, new QTableWidgetItem(QString::fromStdString(datetime::toString(f->getArrival()))));
        flightsTable->setItem(r, 5, new QTableWidgetItem(QString::fromStdString(Flight::statusToString(f->getStatus()))));
        flightsTable->setItem(r, 6, new QTableWidgetItem(QString::number((int)f->getAvailableSeats().size())));
    }

    fId->setText(QString::fromStdString(ctx->flightRepo.getNextId()));
}

void AdminDashboard::addFlight() {
    if (fFrom->text().isEmpty() || fTo->text().isEmpty() ||
        fDepDate->text().isEmpty() || fDepTime->text().isEmpty() ||
        fArrDate->text().isEmpty() || fArrTime->text().isEmpty()) {
        QMessageBox::warning(this, "Missing Fields",
                             "Please fill: From, To, both dates and times.");
        return;
    }

    if (fEcoSeats->value() == 0 && fBizSeats->value() == 0) {
        QMessageBox::warning(this, "No Seats",
                             "A flight must have at least one Economy or Business seat.");
        return;
    }
    if (fEcoSeats->value() > 0 && fEcoPrice->value() == 0) {
        QMessageBox::warning(this, "Missing Price",
                             "Economy seats are defined but Economy price is 0.\n"
                             "Please enter a valid Economy price.");
        return;
    }
    if (fBizSeats->value() > 0 && fBizPrice->value() == 0) {
        QMessageBox::warning(this, "Missing Price",
                             "Business seats are defined but Business price is 0.\n"
                             "Please enter a valid Business price.");
        return;
    }

    try {
        std::string depStr = fDepDate->date().toString("dd-MM-yyyy").toStdString()
        + " " + fDepTime->time().toString("HH:mm").toStdString();
        std::string arrStr = fArrDate->date().toString("dd-MM-yyyy").toStdString()
                             + " " + fArrTime->time().toString("HH:mm").toStdString();

        datetime dep = datetime::fromString(depStr);
        datetime arr = datetime::fromString(arrStr);

        if (!(dep < arr)) {
            QMessageBox::warning(this, "Invalid Times",
                                 "Arrival date/time must be after departure date/time.");
            return;
        }

        Flight newFlight(
            ctx->flightRepo.getNextId(),
            fFrom->text().toStdString(),
            fTo->text().toStdString(),
            dep, arr,
            fEcoSeats->value(),
            fBizSeats->value(),
            fEcoPrice->value(),
            fBizPrice->value(),
            Flight::FlightStatus::Scheduled);

        ctx->flightRepo.addFlight(newFlight);
        ctx->flightRepo.save();
        loadFlightsTable();

        fFrom->clear(); fTo->clear();
        fDepDate->setDate(QDate::currentDate());
        fDepTime->setTime(QTime(0, 0));
        fArrDate->setDate(QDate::currentDate());
        fArrTime->setTime(QTime(1, 0));
        fEcoSeats->setValue(0); fBizSeats->setValue(0);
        fEcoPrice->setValue(0); fBizPrice->setValue(0);

        QMessageBox::information(this, "Success", "Flight added and saved.");
    }
    catch (const std::exception& e) {
        QMessageBox::critical(this, "Error",
                              QString("Invalid input:\n") + e.what());
    }
}

void AdminDashboard::deleteFlight() {
    int row = flightsTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Error", "Select a flight first.");
        return;
    }
    std::string id     = flightsTable->item(row, 0)->text().toStdString();
    std::string status = flightsTable->item(row, 5)->text().toStdString();

    if (status == "Departed" || status == "Arrived") {
        QMessageBox::warning(this, "Cannot Delete",
                             "Flight " + QString::fromStdString(id) +
                                 " has already " + QString::fromStdString(status) + ".\n"
                                                                                    "Departed or Arrived flights cannot be deleted.");
        return;
    }

    bool hasBookings = false;
    for (auto* b : ctx->bookingRepo.share()) {
        if (b->getFlightId() == id &&
            b->getStatus() == Booking::BookingStatus::Booked) {
            hasBookings = true;
            break;
        }
    }
    if (hasBookings) {
        auto reply = QMessageBox::question(
            this, "Active Bookings",
            "Flight " + QString::fromStdString(id) +
                " has active bookings.\nDeleting it will leave those bookings orphaned.\n\n"
                "Are you sure you want to delete it?",
            QMessageBox::Yes | QMessageBox::No);
        if (reply != QMessageBox::Yes) return;
    }

    auto& flights = ctx->flightRepo.share();
    auto it = std::find_if(flights.begin(), flights.end(),
                           [&](Flight* f) { return f->getId() == id; });
    if (it != flights.end()) {
        delete *it;
        flights.erase(it);
        ctx->flightRepo.save();
        loadFlightsTable();
        QMessageBox::information(this, "Deleted",
                                 "Flight " + QString::fromStdString(id) + " removed.");
    }
}

// ── updateFlightStatus  (full replacement) ────────────────────────────────
// Drop-in replacement for the existing AdminDashboard::updateFlightStatus()
// Only ONE new block is added: the "Delayed → Scheduled" reschedule dialog.
// Everything else is identical to the original.

void AdminDashboard::updateFlightStatus()
{
    int row = flightsTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Error", "Select a flight first.");
        return;
    }

    std::string id        = flightsTable->item(row, 0)->text().toStdString();
    std::string curStatus = flightsTable->item(row, 5)->text().toStdString();

    const Flight::FlightStatus statuses[] = {
        Flight::FlightStatus::Scheduled,
        Flight::FlightStatus::Delayed,
        Flight::FlightStatus::Cancelled,
        Flight::FlightStatus::Departed,
        Flight::FlightStatus::Arrived
    };
    Flight::FlightStatus newStatus    = statuses[fStatus->currentIndex()];
    std::string          newStatusStr = fStatus->currentText().toStdString();

    // ── Existing guard rails ───────────────────────────────────────────────
    if (curStatus == "Arrived") {
        QMessageBox::warning(this, "Cannot Update",
                             "This flight has already Arrived and its status is final.");
        return;
    }
    if (curStatus == "Departed" &&
        (newStatus == Flight::FlightStatus::Scheduled ||
         newStatus == Flight::FlightStatus::Cancelled ||
         newStatus == Flight::FlightStatus::Delayed)) {
        std::string m = "A Departed flight can only be marked as "
                        + Flight::statusToString(newStatus);
        QMessageBox::warning(this, "Invalid Transition",
                             QString::fromStdString(m));
        return;
    }
    if (curStatus == "Departed" && newStatus == Flight::FlightStatus::Cancelled) {
        QMessageBox::warning(this, "Cannot Cancel",
                             "A Departed flight cannot be cancelled.");
        return;
    }
    if (curStatus == "Cancelled" && newStatus == Flight::FlightStatus::Scheduled) {
        QMessageBox::warning(this, "Invalid Transition",
                             "A Cancelled flight cannot be re-Scheduled.\n"
                             "Please add a new flight instead.");
        return;
    }

    if (newStatus == Flight::FlightStatus::Cancelled) {
        bool hasBookings = false;
        for (auto* b : ctx->bookingRepo.share()) {
            if (b->getFlightId() == id &&
                b->getStatus() == Booking::BookingStatus::Booked) {
                hasBookings = true;
                break;
            }
        }
        if (hasBookings) {
            auto reply = QMessageBox::question(
                this, "Active Bookings",
                "This flight has active passenger bookings.\n"
                "Cancelling it will not automatically refund passengers.\n\n"
                "Proceed with cancellation?",
                QMessageBox::Yes | QMessageBox::No);
            if (reply != QMessageBox::Yes) return;
        }
    }

    // ── NEW: Delayed → Scheduled  ──────────────────────────────────────────
    // When rescheduling a delayed flight the admin must supply new times.
    if (curStatus == "Delayed" && newStatus == Flight::FlightStatus::Scheduled)
    {
        Flight* flight = ctx->flightRepo.getFlightById(id);
        if (!flight) {
            QMessageBox::critical(this, "Error", "Flight not found.");
            return;
        }

        // ── Build reschedule dialog ────────────────────────────────────────
        QDialog dlg(this);
        dlg.setWindowTitle("Reschedule Flight  –  " + QString::fromStdString(id));
        dlg.setFixedWidth(460);
        dlg.setStyleSheet(
            "QDialog{ background:#0e0e0e; color:#e5e7eb;"
            "  font-family:'Segoe UI'; font-size:13px; }"
            "QLabel{ color:#9ca3af; }"
            "QLabel#dlgTitle{ color:#f97316; font-size:15px; font-weight:bold; }"
            "QLabel#dlgInfo{ background:#141414; border:1px solid #1f2937;"
            "  border-radius:8px; padding:10px; color:#6b7280; }"
            "QLabel#errLabel{ color:#f87171; font-size:12px; }"
            "QDateEdit, QTimeEdit{"
            "  background:#141414; border:1px solid #1f2937;"
            "  border-radius:6px; padding:7px 10px; color:#f9fafb; }"
            "QDateEdit:focus, QTimeEdit:focus{ border:1px solid #f97316; }"
            "QDateEdit::drop-down{ background:#1f2937; border:none;"
            "  border-radius:0 6px 6px 0; width:28px; }"
            "QDateEdit::drop-down:hover{ background:#374151; }"
            "QCalendarWidget QAbstractItemView{ background:#141414; color:#e5e7eb;"
            "  selection-background-color:#f97316; selection-color:#000; }"
            "QCalendarWidget QWidget#qt_calendar_navigationbar{ background:#0e0e0e; }"
            "QCalendarWidget QToolButton{ color:#f97316; background:transparent; font-weight:bold; }"
            "QCalendarWidget QToolButton:hover{ color:#fb923c; }"
            "QTimeEdit::up-button, QTimeEdit::down-button{"
            "  background:#1f2937; border:none; width:20px; }"
            "QTimeEdit::up-button:hover, QTimeEdit::down-button:hover{ background:#374151; }"
            "QPushButton#confirmBtn{ background:#f97316; color:#000; border:none;"
            "  border-radius:6px; padding:9px 22px; font-weight:bold; }"
            "QPushButton#confirmBtn:hover{ background:#fb923c; }"
            "QPushButton#confirmBtn:pressed{ background:#ea6c0a; }"
            "QPushButton#cancelBtn{ background:#1f2937; color:#e5e7eb; border:none;"
            "  border-radius:6px; padding:9px 22px; font-weight:bold; }"
            "QPushButton#cancelBtn:hover{ background:#374151; }");

        auto* vMain = new QVBoxLayout(&dlg);
        vMain->setContentsMargins(24, 20, 24, 20);
        vMain->setSpacing(12);

        // Title
        auto* titleLbl = new QLabel("🗓  Set New Schedule for Flight "
                                    + QString::fromStdString(id));
        titleLbl->setObjectName("dlgTitle");
        vMain->addWidget(titleLbl);

        // Info box showing current (delayed) times
        auto* infoLbl = new QLabel(
            "Current (delayed) times:\n"
            "  Departure:  "
            + QString::fromStdString(datetime::toString(flight->getDeparture()))
            + "\n  Arrival:    "
            + QString::fromStdString(datetime::toString(flight->getArrival())));
        infoLbl->setObjectName("dlgInfo");
        vMain->addWidget(infoLbl);

        // ── Departure row ──────────────────────────────────────────────────
        auto* depGrp = new QGroupBox("New Departure");
        depGrp->setStyleSheet(
            "QGroupBox{ color:#f97316; border:1px solid #1f2937;"
            "border-radius:8px; margin-top:8px; padding:10px; }"
            "QGroupBox::title{ subcontrol-origin:margin; left:10px; padding:0 4px; }");
        auto* depRow = new QHBoxLayout(depGrp);
        depRow->setSpacing(10);

        auto* newDepDate = new QDateEdit(QDate::currentDate());
        newDepDate->setDisplayFormat("dd-MM-yyyy");
        newDepDate->setCalendarPopup(true);
        newDepDate->setMinimumDate(QDate::currentDate());

        auto* newDepTime = new QTimeEdit(QTime::currentTime().addSecs(3600));
        newDepTime->setDisplayFormat("HH:mm");

        depRow->addWidget(new QLabel("Date:"));
        depRow->addWidget(newDepDate, 1);
        depRow->addSpacing(16);
        depRow->addWidget(new QLabel("Time:"));
        depRow->addWidget(newDepTime, 1);
        vMain->addWidget(depGrp);

        // ── Arrival row ───────────────────────────────────────────────────
        auto* arrGrp = new QGroupBox("New Arrival");
        arrGrp->setStyleSheet(depGrp->styleSheet());
        auto* arrRow = new QHBoxLayout(arrGrp);
        arrRow->setSpacing(10);

        auto* newArrDate = new QDateEdit(QDate::currentDate());
        newArrDate->setDisplayFormat("dd-MM-yyyy");
        newArrDate->setCalendarPopup(true);
        newArrDate->setMinimumDate(QDate::currentDate());

        auto* newArrTime = new QTimeEdit(QTime::currentTime().addSecs(7200));
        newArrTime->setDisplayFormat("HH:mm");

        arrRow->addWidget(new QLabel("Date:"));
        arrRow->addWidget(newArrDate, 1);
        arrRow->addSpacing(16);
        arrRow->addWidget(new QLabel("Time:"));
        arrRow->addWidget(newArrTime, 1);
        vMain->addWidget(arrGrp);

        // ── Validation error label ────────────────────────────────────────
        auto* errLbl = new QLabel("");
        errLbl->setObjectName("errLabel");
        errLbl->setWordWrap(true);
        errLbl->hide();
        vMain->addWidget(errLbl);

        // ── Buttons ───────────────────────────────────────────────────────
        auto* btnRow = new QHBoxLayout;
        btnRow->addStretch();
        auto* cancelDlgBtn = new QPushButton("Cancel");
        cancelDlgBtn->setObjectName("cancelBtn");
        cancelDlgBtn->setCursor(Qt::PointingHandCursor);
        auto* confirmBtn = new QPushButton("✅  Reschedule & Set Scheduled");
        confirmBtn->setObjectName("confirmBtn");
        confirmBtn->setCursor(Qt::PointingHandCursor);
        btnRow->addWidget(cancelDlgBtn);
        btnRow->addWidget(confirmBtn);
        vMain->addLayout(btnRow);

        connect(cancelDlgBtn, &QPushButton::clicked, &dlg, &QDialog::reject);

        // ── Confirm: validate then apply ──────────────────────────────────
        connect(confirmBtn, &QPushButton::clicked, [&]()
                {
                    // Build datetime strings from the four widgets
                    std::string depStr =
                        newDepDate->date().toString("dd-MM-yyyy").toStdString()
                        + " "
                        + newDepTime->time().toString("HH:mm").toStdString();

                    std::string arrStr =
                        newArrDate->date().toString("dd-MM-yyyy").toStdString()
                        + " "
                        + newArrTime->time().toString("HH:mm").toStdString();

                    datetime newDep, newArr;
                    try {
                        newDep = datetime::fromString(depStr);
                        newArr = datetime::fromString(arrStr);
                    } catch (const std::exception& e) {
                        errLbl->setText("⚠  " + QString(e.what()));
                        errLbl->show();
                        return;
                    }

                    // ── Validation 1: departure must be in the future ─────────────
                    datetime now = datetime::now();   // assumes your datetime has ::now()
                    if (!(now < newDep)) {
                        errLbl->setText(
                            "⚠  Departure must be in the future.\n"
                            "    Please choose a date/time after right now.");
                        errLbl->show();
                        return;
                    }

                    // ── Validation 2: arrival must be in the future ───────────────
                    if (!(now < newArr)) {
                        errLbl->setText(
                            "⚠  Arrival must be in the future.\n"
                            "    Please choose a date/time after right now.");
                        errLbl->show();
                        return;
                    }

                    // ── Validation 3: arrival must be strictly after departure ────
                    if (!(newDep < newArr)) {
                        errLbl->setText(
                            "⚠  Arrival must be after Departure.\n"
                            "    Please correct the times.");
                        errLbl->show();
                        return;
                    }

                    // ── All good: update the flight object ────────────────────────
                    flight->setDeparture(newDep);
                    flight->setArrival(newArr);

                    dlg.accept();   // close the reschedule dialog and continue below
                });

        // Show dialog; if admin cancelled, abort the whole status update
        if (dlg.exec() != QDialog::Accepted)
            return;
    }
    // ── END new block ──────────────────────────────────────────────────────

    // Apply the status change (works for all transitions, including the
    // Delayed→Scheduled path whose times were already updated above)
    if (ctx->flightRepo.updateFlightStatus(id, newStatus)) {
        ctx->flightRepo.save();
        loadFlightsTable();
        QMessageBox::information(
            this, "Updated",
            "Flight " + QString::fromStdString(id) +
                " status updated to " +
                QString::fromStdString(newStatusStr) + ".");
    }
}
// ── All Bookings ───────────────────────────────────────────────────────────
QWidget* AdminDashboard::buildBookingsPage() {
    auto* w = new QWidget;
    auto* v = new QVBoxLayout(w);
    v->setContentsMargins(20, 20, 20, 20);
    v->setSpacing(10);

    auto* title = new QLabel("All Bookings");
    title->setObjectName("pageTitle");
    v->addWidget(title);

    auto* hint = new QLabel("💡 Click a User ID to jump to that user's profile and bookings");
    hint->setObjectName("infoLabel");
    v->addWidget(hint);

    bookingsTable = new QTableWidget(0, 6);
    bookingsTable->setHorizontalHeaderLabels(
        { "Booking ID","User ID","Flight","Seat","Date","Status" });
    bookingsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    bookingsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    bookingsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    bookingsTable->setAlternatingRowColors(true);

    // Highlight User ID column on hover and make it clickable
    connect(bookingsTable, &QTableWidget::cellEntered,
            [this](int /*row*/, int col) {
                bookingsTable->setCursor(col == 1 ? Qt::PointingHandCursor
                                                  : Qt::ArrowCursor);
            });
    connect(bookingsTable, &QTableWidget::cellClicked,
            [this](int row, int col) {
                if (col != 1) return;
                auto* item = bookingsTable->item(row, 1);
                if (!item) return;
                QString userId = item->text();

                // Look up name from userRepo
                QString userName = userId;
                for (auto* u : ctx->userRepo.getUsers()) {
                    if (QString::fromStdString(u->get_userId()) == userId) {
                        userName = QString::fromStdString(u->getname());
                        break;
                    }
                }

                // Switch to All Users page and highlight the row
                sidebar->setCurrentRow(2);
                loadAllUsers();
                contentStack->setCurrentIndex(2);
                for (int r = 0; r < usersTable->rowCount(); ++r) {
                    if (usersTable->item(r, 0) &&
                        usersTable->item(r, 0)->text() == userId) {
                        usersTable->selectRow(r);
                        usersTable->scrollToItem(usersTable->item(r, 0));
                        break;
                    }
                }

                // Open the same booking-history dialog
                showUserBookings(userId, userName);
            });

    bookingsTable->setMouseTracking(true);
    v->addWidget(bookingsTable);

    return w;
}

void AdminDashboard::loadAllBookings() {
    bookingsTable->setRowCount(0);

    for (auto* b : ctx->bookingRepo.share()) {
        int r = bookingsTable->rowCount();
        bookingsTable->insertRow(r);

        QString status = (b->getStatus() == Booking::BookingStatus::Booked)
                             ? "✅ Booked" : "❌ Cancelled";

        bookingsTable->setItem(r, 0, new QTableWidgetItem(QString::fromStdString(b->getBookingId())));

        // User ID — styled as a clickable link
        auto* userIdItem = new QTableWidgetItem(QString::fromStdString(b->getUserId()));
        userIdItem->setForeground(QColor("#f97316"));
        QFont linkFont = userIdItem->font();
        linkFont.setUnderline(true);
        userIdItem->setFont(linkFont);
        userIdItem->setToolTip("Click to view this user's profile and bookings");
        bookingsTable->setItem(r, 1, userIdItem);

        bookingsTable->setItem(r, 2, new QTableWidgetItem(QString::fromStdString(b->getFlightId())));
        bookingsTable->setItem(r, 3, new QTableWidgetItem(QString::fromStdString(b->getSeatNumber())));
        bookingsTable->setItem(r, 4, new QTableWidgetItem(QString::fromStdString(datetime::toString(b->getBookingDate()))));
        bookingsTable->setItem(r, 5, new QTableWidgetItem(status));
    }
}

// ── All Users ──────────────────────────────────────────────────────────────
QWidget* AdminDashboard::buildUsersPage() {
    auto* w = new QWidget;
    auto* v = new QVBoxLayout(w);
    v->setContentsMargins(20, 20, 20, 20);
    v->setSpacing(10);

    auto* title = new QLabel("All Users");
    title->setObjectName("pageTitle");
    v->addWidget(title);

    auto* hint = new QLabel("💡 Double-click a user row to view their booking history");
    hint->setObjectName("infoLabel");
    v->addWidget(hint);

    usersTable = new QTableWidget(0, 3);
    usersTable->setHorizontalHeaderLabels({ "User ID", "Name", "Email" });
    usersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    usersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    usersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    usersTable->setAlternatingRowColors(true);

    connect(usersTable, &QTableWidget::cellDoubleClicked,
            [this](int row, int /*col*/) {
                QString userId   = usersTable->item(row, 0)->text();
                QString userName = usersTable->item(row, 1)->text();
                showUserBookings(userId, userName);
            });

    v->addWidget(usersTable);
    return w;
}

void AdminDashboard::loadAllUsers() {
    usersTable->setRowCount(0);

    for (auto* u : ctx->userRepo.getUsers()) {
        int r = usersTable->rowCount();
        usersTable->insertRow(r);
        usersTable->setItem(r, 0, new QTableWidgetItem(QString::fromStdString(u->get_userId())));
        usersTable->setItem(r, 1, new QTableWidgetItem(QString::fromStdString(u->getname())));
        usersTable->setItem(r, 2, new QTableWidgetItem(QString::fromStdString(u->getemail())));
    }
}

void AdminDashboard::showUserBookings(const QString& userId, const QString& userName)
{
    QDialog dlg(this);
    dlg.setWindowTitle("Bookings for " + userName + " (" + userId + ")");
    dlg.setMinimumSize(780, 480);
    dlg.setStyleSheet(
        "QDialog{ background:#0e0e0e; color:#e5e7eb;"
        "  font-family:'Segoe UI'; font-size:13px; }"
        "QTableWidget{ background:#0e0e0e; border:none;"
        "  gridline-color:#1f2937; color:#e5e7eb;"
        "  alternate-background-color:#111111; }"
        "QHeaderView::section{ background:#141414; color:#f97316;"
        "  padding:8px; border:none;"
        "  border-bottom:2px solid #f97316; font-weight:bold; }"
        "QHeaderView::section:hover{ background:#1a1a1a; color:#fb923c; }"
        "QTableWidget::item{ padding:4px 6px; }"
        "QTableWidget::item:hover{ background:#1a1a1a; color:#f9fafb; }"
        "QTableWidget::item:selected{ background:#1a0d00; color:#ffffff;"
        "  border-left:3px solid #f97316; }"
        "QLabel#dlgTitle{ font-size:18px; font-weight:bold; color:#f97316; }"
        "QLabel#dlgInfo{ background:#141414; border:1px solid #1f2937;"
        "  border-radius:8px; padding:10px; color:#6b7280; }"
        "QPushButton{ background:#f97316; color:#000000; border:none;"
        "  border-radius:6px; padding:9px 22px; font-weight:bold; }"
        "QPushButton:hover{ background:#fb923c; }"
        "QPushButton:pressed{ background:#ea6c0a; }");

    auto* v = new QVBoxLayout(&dlg);
    v->setContentsMargins(20, 20, 20, 20);
    v->setSpacing(10);

    auto* ttl = new QLabel("📋  Booking History — " + userName);
    ttl->setObjectName("dlgTitle");
    v->addWidget(ttl);

    auto* info = new QLabel("User ID: " + userId);
    info->setObjectName("dlgInfo");
    v->addWidget(info);

    auto* table = new QTableWidget(0, 6);
    table->setHorizontalHeaderLabels(
        { "Booking ID", "Flight", "From", "To", "Seat", "Status" });
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);

    auto history = ctx->bookingRepo.getUserHistory(userId.toStdString());
    for (auto* b : history) {
        int r = table->rowCount();
        table->insertRow(r);

        Flight* f  = ctx->flightRepo.getFlightById(b->getFlightId());
        QString from = f ? QString::fromStdString(f->getSource())      : "—";
        QString to   = f ? QString::fromStdString(f->getDestination()) : "—";

        QString status = (b->getStatus() == Booking::BookingStatus::Booked)
                             ? "✅ Booked" : "❌ Cancelled";

        // Store Booking* in UserRole of col 0 for the double-click handler
        auto* idItem = new QTableWidgetItem(QString::fromStdString(b->getBookingId()));
        idItem->setData(Qt::UserRole,
                        QVariant::fromValue<quintptr>(reinterpret_cast<quintptr>(b)));
        table->setItem(r, 0, idItem);
        table->setItem(r, 1, new QTableWidgetItem(QString::fromStdString(b->getFlightId())));
        table->setItem(r, 2, new QTableWidgetItem(from));
        table->setItem(r, 3, new QTableWidgetItem(to));
        table->setItem(r, 4, new QTableWidgetItem(QString::fromStdString(b->getSeatNumber())));
        table->setItem(r, 5, new QTableWidgetItem(status));
    }

    // Double-click → receipt dialog
    connect(table, &QTableWidget::cellDoubleClicked,
            [this, &dlg, table](int row, int /*col*/) {
                auto* item = table->item(row, 0);
                if (!item) return;
                Booking* b = reinterpret_cast<Booking*>(
                    item->data(Qt::UserRole).value<quintptr>());
                if (!b) return;

                bool cancelled = (b->getStatus() == Booking::BookingStatus::Cancelled);

                Flight* f   = ctx->flightRepo.getFlightById(b->getFlightId());
                QString from = f ? QString::fromStdString(f->getSource())      : "—";
                QString to   = f ? QString::fromStdString(f->getDestination()) : "—";
                QString dep  = f ? QString::fromStdString(datetime::toString(f->getDeparture())) : "—";
                QString arr  = f ? QString::fromStdString(datetime::toString(f->getArrival()))   : "—";

                QString seatNo = QString::fromStdString(b->getSeatNumber());
                bool    isBiz  = !seatNo.isEmpty() && seatNo[0] == 'B';

                // Walk receipt → payment chain
                QString bank="—", method="—", maskedAcc="—",
                    txnId="—", receiptId="—";
                double  amount = 0.0;

                Receipt* receipt = ctx->receiptRepo.searchByBookingId(b->getBookingId());
                if (receipt) {
                    receiptId = QString::fromStdString(receipt->getReceiptId());
                    txnId     = QString::fromStdString(receipt->getPaymentId());
                    Payment* pay = ctx->paymentRepo.getPaymentById(txnId.toStdString());
                    if (pay) {
                        bank   = QString::fromStdString(pay->getBankName());
                        method = QString::fromStdString(pay->getPaymentMethod());
                        amount = pay->getAmount();
                        QString acc = QString::fromStdString(pay->getAccountNumber());
                        maskedAcc = acc.length() > 4
                                        ? QString("*").repeated(acc.length()-4) + acc.right(4)
                                        : acc;
                    }
                }

                QString bookingDate = QString::fromStdString(
                    datetime::toString(b->getBookingDate()));

                // Lookup user name
                QString uName = "—";
                for (auto* u : ctx->userRepo.getUsers()) {
                    if (QString::fromStdString(u->get_userId()) ==
                        QString::fromStdString(b->getUserId())) {
                        uName = QString::fromStdString(u->getname());
                        break;
                    }
                }

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
  <div style='background:#0e0e0e; border:1px solid #1f2937;
              border-radius:12px; padding:20px;
              text-align:center; margin-bottom:16px;'>
    <div style='color:#f97316; font-size:24px; font-weight:bold; margin:6px 0;'>✈ AeroSync</div>
    <div style='color:#6b7280; font-size:11px; letter-spacing:3px;'>BOOKING RECEIPT</div>
  </div>
  <div style='background:%1; border:1px solid %2;
              border-radius:8px; padding:10px 16px;
              text-align:center; margin-bottom:16px;'>
    <span style='color:%3; font-size:14px; font-weight:bold;'>%4 %5</span>
  </div>
  <table width='100%%' cellspacing='8' cellpadding='0'><tr>
    <td width='50%%' valign='top'>
      <div style='background:#0e0e0e; border:1px solid #1f2937;
                  border-radius:10px; padding:14px;'>
        <div style='color:#f97316; font-weight:bold; font-size:13px;
                    margin-bottom:10px; border-bottom:1px solid #1f2937; padding-bottom:6px;'>
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
    <td width='50%%' valign='top'>
      <div style='background:#0e0e0e; border:1px solid #1f2937;
                  border-radius:10px; padding:14px;'>
        <div style='color:#f97316; font-weight:bold; font-size:13px;
                    margin-bottom:10px; border-bottom:1px solid #1f2937; padding-bottom:6px;'>
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
  </tr></table>
  <div style='background:#0e0e0e; border:1px solid #f97316;
              border-radius:10px; padding:14px 20px;
              text-align:center; margin-top:12px;'>
    <span style='color:#6b7280; font-size:13px;'>Total Amount</span><br>
    <span style='color:#f97316; font-size:24px; font-weight:bold;'>PKR %24</span>
  </div>
  <div style='background:#0e0e0e; border:1px solid #1f2937;
              border-radius:10px; padding:12px 16px; margin-top:12px;'>
    <span style='color:#6b7280;'>Passenger: </span>
    <span style='color:#e5e7eb; font-weight:bold;'>%25</span>
    &nbsp;&nbsp;
    <span style='color:#6b7280;'>User ID: </span>
    <span style='color:#e5e7eb;'>%26</span>
  </div>
  <div style='text-align:center; color:#374151; font-size:11px; margin-top:14px;'>
    ✈ Thank you for flying with AeroSync
  </div>
</div>)")
                                   .arg(statusBg).arg(statusBorder).arg(statusColor)   // 1 2 3
                                   .arg(statusIcon).arg(statusText)                     // 4 5
                                   .arg(QString::fromStdString(b->getFlightId()))       // 6
                                   .arg(from).arg(to)                                   // 7 8
                                   .arg(dep).arg(arr)                                   // 9 10
                                   .arg(seatNo)                                         // 11
                                   .arg(isBiz ? "#a78bfa" : "#fb923c")                 // 12
                                   .arg(isBiz ? "Business" : "Economy")                 // 13
                                   .arg(bookingDate)                                    // 14
                                   .arg(bookingLabel)                                   // 15
                                   .arg(receiptId)                                      // 16
                                   .arg(QString::fromStdString(b->getBookingId()))      // 17
                                   .arg(txnId)                                          // 18
                                   .arg(bank).arg(method).arg(maskedAcc)               // 19 20 21
                                   .arg(payColor).arg(payLabel)                         // 22 23
                                   .arg(amount, 0, 'f', 2)                             // 24
                                   .arg(uName)                                          // 25
                                   .arg(QString::fromStdString(b->getUserId()));        // 26

                QDialog rdlg(&dlg);
                rdlg.setWindowTitle("Receipt — " +
                                    QString::fromStdString(b->getBookingId()));
                rdlg.setMinimumSize(700, 580);
                rdlg.setStyleSheet(
                    "QDialog{ background:#141414; }"
                    "QTextEdit{ background:#141414; border:none; }"
                    "QPushButton{ background:#f97316; color:#000000; border:none;"
                    "  border-radius:6px; padding:9px 22px; font-weight:bold; }"
                    "QPushButton:hover{ background:#fb923c; }"
                    "QPushButton:pressed{ background:#ea6c0a; }");

                auto* vr  = new QVBoxLayout(&rdlg);
                auto* te  = new QTextEdit;
                te->setReadOnly(true);
                te->setHtml(html);
                vr->addWidget(te);

                auto* rClose = new QPushButton("Close");
                rClose->setCursor(Qt::PointingHandCursor);
                connect(rClose, &QPushButton::clicked, &rdlg, &QDialog::accept);
                vr->addWidget(rClose, 0, Qt::AlignRight);

                rdlg.exec();
            });

    if (table->rowCount() == 0) {
        auto* noData = new QLabel("  No bookings found for this user.");
        noData->setObjectName("dlgInfo");
        v->addWidget(noData);
    }

    auto* hintLbl = new QLabel("💡 Double-click a booking row to view its receipt");
    hintLbl->setObjectName("dlgInfo");
    v->addWidget(hintLbl);

    v->addWidget(table);

    auto* closeBtn = new QPushButton("Close");
    closeBtn->setCursor(Qt::PointingHandCursor);
    connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    v->addWidget(closeBtn, 0, Qt::AlignRight);

    dlg.exec();
}

// ── Profit Calculator ──────────────────────────────────────────────────────
QWidget* AdminDashboard::buildProfitPage() {
    auto* w = new QWidget;
    auto* v = new QVBoxLayout(w);
    v->setContentsMargins(80, 40, 80, 40);
    v->setSpacing(14);

    auto* title = new QLabel("Profit Calculator");
    title->setObjectName("pageTitle");
    v->addWidget(title);

    auto* note = new QLabel(
        "Revenue is computed from all confirmed bookings.\n"
        "Enter your total operational cost and click Calculate.");
    note->setObjectName("infoLabel");
    v->addWidget(note);

    v->addSpacing(10);
    v->addWidget(new QLabel("Total Operational Cost (PKR):"));
    costInput = new QDoubleSpinBox;
    costInput->setRange(0, 999'999'999);
    costInput->setDecimals(2);
    costInput->setPrefix("PKR  ");
    costInput->setValue(0);
    v->addWidget(costInput);

    auto* calcBtn = new QPushButton("Calculate Profit / Loss");
    calcBtn->setObjectName("actionBtn");
    calcBtn->setCursor(Qt::PointingHandCursor);
    connect(calcBtn, &QPushButton::clicked, this, &AdminDashboard::calculateProfit);
    v->addSpacing(8);
    v->addWidget(calcBtn);

    profitResult = new QLabel("");
    profitResult->setObjectName("profitBox");
    profitResult->setAlignment(Qt::AlignCenter);
    profitResult->setWordWrap(true);
    v->addSpacing(20);
    v->addWidget(profitResult);
    v->addStretch();
    return w;
}

void AdminDashboard::calculateProfit() {
    double cost    = costInput->value();
    double revenue = 0.0;
    int    tickets = 0;

    for (auto* b : ctx->bookingRepo.share()) {
        if (b->getStatus() != Booking::BookingStatus::Booked) continue;
        tickets++;

        Flight* f = ctx->flightRepo.getFlightById(b->getFlightId());
        if (!f) continue;

        std::string seatNo = b->getSeatNumber();
        bool isBiz = !seatNo.empty() && seatNo[0] == 'B';
        Seat::SeatClass cls = isBiz ? Seat::SeatClass::Business
                                    : Seat::SeatClass::Economy;

        bool found = false;
        for (auto* s : f->getAvailableSeats()) {
            if (s->getSeatClass() == cls) { revenue += s->getPrice(); found = true; break; }
        }
        if (!found) {
            for (auto* s : f->getReservedSeats()) {
                if (s->getSeatClass() == cls) { revenue += s->getPrice(); break; }
            }
        }
    }

    ProfitCalculator pc(cost, revenue, tickets);
    double profit   = pc.calcProfit();
    bool profitable = pc.isProfitable();

    QString color = profitable ? "#fb923c" : "#ef5350";  // orange for profit, red for loss
    QString sign  = profitable ? "PROFIT +" : "LOSS  -";

    profitResult->setText(QString(
                              "<b>Confirmed Tickets Sold:</b>  %1<br>"
                              "<b>Total Revenue:</b>  PKR %2<br>"
                              "<b>Total Cost:</b>  PKR %3<br><br>"
                              "<span style='font-size:20px;color:%4;'><b>%5 PKR %6</b></span><br>"
                              "<span style='color:%4;font-size:13px;'>%7</span>")
                              .arg(tickets)
                              .arg(revenue, 0, 'f', 2)
                              .arg(cost, 0, 'f', 2)
                              .arg(color, sign)
                              .arg(QString::number(std::abs(profit), 'f', 2))
                              .arg(profitable ? "📈 Operations are profitable"
                                              : "📉 Operations are running at a loss"));
}

void AdminDashboard::logout() {
    close();
    (new LoginWindow(ctx))->show();
}

void AdminDashboard::applyStyles() {
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
        QGroupBox#grp {
            background:#0e0e0e; border:1px solid #1f2937;
            border-radius:8px; padding:10px; margin-top:6px;
            color:#6b7280; font-weight:bold;
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
        QLineEdit, QComboBox, QSpinBox,
        QDoubleSpinBox, QDateEdit, QTimeEdit {
            background:#141414; border:1px solid #1f2937;
            border-radius:6px; padding:7px 10px; color:#f9fafb;
        }
        QLineEdit:focus, QComboBox:focus,
        QSpinBox:focus, QDoubleSpinBox:focus,
        QDateEdit:focus, QTimeEdit:focus {
            border:1px solid #f97316;
        }

        /* ── Spin box up/down buttons ──────────────────────── */
        QSpinBox::up-button, QDoubleSpinBox::up-button,
        QTimeEdit::up-button {
            background:#1f2937; border:none;
            border-radius:0 4px 0 0; width:20px;
        }
        QSpinBox::down-button, QDoubleSpinBox::down-button,
        QTimeEdit::down-button {
            background:#1f2937; border:none;
            border-radius:0 0 4px 0; width:20px;
        }
        QSpinBox::up-button:hover, QDoubleSpinBox::up-button:hover,
        QTimeEdit::up-button:hover {
            background:#374151;
        }
        QSpinBox::down-button:hover, QDoubleSpinBox::down-button:hover,
        QTimeEdit::down-button:hover {
            background:#374151;
        }

        /* ── Date picker calendar button ───────────────────── */
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
        QPushButton#dangerBtn {
            background:#1f2937; color:#e5e7eb; border:none;
            border-radius:6px; padding:9px 22px; font-weight:bold;
        }
        QPushButton#dangerBtn:hover { background:#7f1d1d; color:white; }
        QPushButton#dangerBtn:pressed { background:#991b1b; }
        QLabel#profitBox {
            background:#141414; border:1px solid #1f2937;
            border-radius:12px; padding:24px; font-size:14px;
        }
    )");
}