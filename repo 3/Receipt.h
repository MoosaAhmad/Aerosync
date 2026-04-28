
#pragma once
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>

#include "Booking.h"
#include "Payment.h"
#include "DateTime.h"

using namespace std;

class Receipt {
private:
    string   receiptId;
    Booking* booking;
    Payment* payment;
    datetime issueDateTime;

public:
    // Constructor  matches exactly what Payment.h calls
    //    Receipt newReceipt(rId, currentBooking, this, currentTime);
    Receipt(string rId, Booking* b, Payment* p, datetime dt)
        : receiptId(rId), booking(b), payment(p), issueDateTime(dt) {
    }

    //  Getters 
    string   getReceiptId()     const { return receiptId; }
    Booking* getBooking()       const { return booking; }
    Payment* getPayment()       const { return payment; }
    datetime getIssueDateTime() const { return issueDateTime; }

    // display

    void display() const {
        cout << "========================================\n";
        cout << "         SKYLINE AIRLINES               \n";
        cout << "      OFFICIAL BOOKING RECEIPT          \n";
        cout << "========================================\n";
        cout << "Receipt ID   : " << receiptId << "\n";
        cout << "Issued On    : " << datetime::toString(issueDateTime) << "\n";

        cout << "----------------------------------------\n";
        cout << "         PASSENGER DETAILS              \n";
        cout << "----------------------------------------\n";
        if (booking && booking->getuser()) {
            User* u = booking->getuser();
            cout << "Name         : " << u->getName() << "\n";
            cout << "User ID      : " << u->getUserId() << "\n";
        }

        cout << "----------------------------------------\n";
        cout << "          BOOKING DETAILS               \n";
        cout << "----------------------------------------\n";
        if (booking) {
            cout << "Booking ID   : " << booking->getBookingId() << "\n";
            cout << "Status       : " << booking->getstatus() << "\n";
        }

        if (booking && booking->getFlight()) {
            Flight* f = booking->getFlight();
            cout << "----------------------------------------\n";
            cout << "          FLIGHT DETAILS                \n";
            cout << "----------------------------------------\n";
            cout << "Flight ID    : " << f->getId() << "\n";
            cout << "From         : " << f->getSource() << "\n";
            cout << "To           : " << f->getDestination() << "\n";
            cout << "Departure    : " << datetime::toString(f->getDeparture()) << "\n";
            cout << "Arrival      : " << datetime::toString(f->getArrival()) << "\n";
            cout << "Status       : " << Flight::statusToString(f->getStatus()) << "\n";
        }

        if (booking && booking->getSeat()) {
            Seat* s = booking->getSeat();
            cout << "----------------------------------------\n";
            cout << "           SEAT DETAILS                 \n";
            cout << "----------------------------------------\n";
            cout << "Seat Number  : " << s->getSeatNumber() << "\n";
            cout << "Class        : " << Seat::classToString(s->getSeatClass()) << "\n";
        }

        if (payment) {
            double base = payment->getAmount();
            double tax = base * 0.15;
            double total = base + tax;

            cout << "----------------------------------------\n";
            cout << "          PAYMENT DETAILS               \n";
            cout << "----------------------------------------\n";
            cout << "Method       : " << payment->getPaymentMethod() << "\n";
            cout << "Bank         : " << payment->getBankName() << "\n";
            cout << "Transaction  : " << payment->getTransactionId() << "\n";
            cout << fixed << setprecision(2);
            cout << "Base Fare    : PKR " << base << "\n";
            cout << "Tax (15%)    : PKR " << tax << "\n";
            cout << "----------------------------------------\n";
            cout << "TOTAL AMOUNT : PKR " << total << "\n";
        }

        cout << "========================================\n";
        cout << "  Thank you for choosing Skyline Airlines!\n";
        cout << "========================================\n";
    }
};