#ifndef ADMINDASHBOARD_H
#define ADMINDASHBOARD_H
#pragma once

#include <QMainWindow>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QStackedWidget>
#include <QListWidget>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QDateEdit>
#include <QTimeEdit>
#include <QEvent>
#include <QGroupBox>
#include"Admin.h"

#include "AppContext.h"
#include "ChangePasswordDialog.h"   // ← new

class AdminDashboard : public QMainWindow {
    Q_OBJECT
public:
    explicit AdminDashboard(Admin* currentAdmin,AppContext* ctx, QWidget* parent = nullptr);

private slots:
    void onSidebarClicked(int row);
    void loadFlightsTable();
    void addFlight();
    void deleteFlight();
    void updateFlightStatus();
    void loadAllBookings();
    void loadAllUsers();
    void calculateProfit();
    void openChangePassword();       // ← new
    void logout();

    // ... existing slots ...
    void onFlightDoubleClicked(int row, int col);  // ← ADD
    void showSeatMap(Flight* flight);               // ← ADD
    void showUserBookings(const QString& userId, const QString& userName); // ← ADD

public:
    // Exposed for inline event-filter access inside showSeatMap
    AppContext*     ctx;
    QStackedWidget* contentStack;
    QListWidget*    sidebar;
    QTableWidget*   usersTable;

private:
    Admin* currentAdmin;

    // Manage Flights
    QTableWidget*   flightsTable;
    QLineEdit*      fId, *fFrom, *fTo;
    QDateEdit*      fDepDate, *fArrDate;
    QTimeEdit*      fDepTime, *fArrTime;
    QSpinBox*       fEcoSeats, *fBizSeats;
    QDoubleSpinBox* fEcoPrice, *fBizPrice;
    QComboBox*      fStatus;

    // All Bookings
    QTableWidget*   bookingsTable;

    // Profit Calculator
    QDoubleSpinBox* costInput;
    QLabel*         profitResult;

    QWidget* buildFlightsPage();
    QWidget* buildBookingsPage();
    QWidget* buildUsersPage();
    QWidget* buildProfitPage();
    void     applyStyles();
};

#endif // ADMINDASHBOARD_H