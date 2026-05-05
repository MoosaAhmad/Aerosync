#ifndef USERDASHBOARD_H
#define USERDASHBOARD_H
#pragma once
#include <QMainWindow>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QStackedWidget>
#include <QListWidget>
#include <QTextEdit>
#include <QDateEdit>
#include <QCheckBox>
#include "AppContext.h"
#include "User.h"
#include "Flight.h"
#include "Booking.h"
#include "ChangePasswordDialog.h"
class UserDashboard : public QMainWindow {
    Q_OBJECT
public:
    explicit UserDashboard(user* currentUser, AppContext* ctx,
                           QWidget* parent = nullptr);
private slots:
    void onSidebarClicked(int row);
    void searchFlights();
    void onFlightDoubleClicked(int row, int col);
    void loadMyBookings();
    void cancelBooking(Booking* b);
    void processPayment();
    void openChangePassword();
    void logout();
private:
    user*       currentUser;
    AppContext* ctx;
    // Pending booking state (set when user picks a seat)
    std::string pendingFlightId;
    std::string pendingSeatNo;
    double      pendingPrice = 0.0;
    QStackedWidget* contentStack;
    QListWidget*    sidebar;
    // Search page
    QLineEdit*    searchFrom;
    QLineEdit*    searchTo;
    QDateEdit*    searchDate;
    QCheckBox*    dateFilterCheck;
    QTableWidget* searchTable;
    QLabel*       searchHint;
    // My Bookings page
    QTableWidget* myBookingsTable;
    // Payment page
    QLabel*    paymentSummary;
    QComboBox* bankCombo;
    QComboBox* methodCombo;
    QLineEdit* accountNumber;
    // Receipt page
    QTextEdit* receiptDisplay;
    QWidget* buildSearchPage();
    QWidget* buildBookingsPage();
    QWidget* buildPaymentPage();
    QWidget* buildReceiptPage();
    void     showSeatPicker(Flight* flight);
    void     applyStyles();
};
#endif // USERDASHBOARD_H