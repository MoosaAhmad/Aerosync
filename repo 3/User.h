#pragma once
#include "Person.h"
#include "Booking.h"
#include "Flight.h"
#include "Vector.h"
#include <string>
#include <iostream>
using namespace std;

class User : public Person {
private:
    string userId;
    Vector<Booking*> history;

public:
    User(string name, string email, string password, string userId)
        : Person(name, email, password), userId(userId) {
    }
    ~User() {
        history.clear();
    }

    // Overriding pure virtual from Person
    void authenticate() override {
        string entered;
        cout << "Enter password for " << name << ": ";
        cin >> entered;

        if (entered == passwordHash)
            cout << "Authentication successful. Welcome, " << name << "!\n";
        else
            cout << "Incorrect password. Access denied.\n";
    }

    // Core Methods

    void searchFlight(const string& origin, const string& destination) {
         // iski samajh nahi aa rahi isme kiya karna :(
    }

    void bookFlight(Booking* b) {
        if (!b) {
            cout << "Invalid booking.\n";
            return;
        }
        b->confirm();
        history.push_back(b);
        cout << "Booking confirmed and added to history.\n";
    }

    void cancelTicket(const string& bookingId) {
        int sz= history.size();
        for (Booking* b, int i = 0; i < sz;++i) {
            if (b->getBookingId() == bookingId) {
                b->cancel();
                cout << "Ticket " << bookingId << " cancelled.\n";
                return;
            }
        }
        cout << "No booking found with ID: " << bookingId << "\n";
    }

    void viewHistory() const {
        if (history.empty()) {
            cout << "No booking history found.\n";
            return;
        }
        cout << "--- Booking History for " << name << " ---\n";
        int sz = history.size();
        for (Booking* b, int i = 0; i < sz;++i)
            b->display();
    }

    void checkFlightStatus(Flight* f) const {
        if (!f) {
            cout << "Invalid flight.\n";
            return;
        }
        f->displayStatus();
    }

    string getUserId() const { return userId; }

    // this will be used by booking class
    string getName() const { return Person::name; }
};