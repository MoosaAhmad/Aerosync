#pragma once

class Seat {
private:
    string seatNumber;
    string seatClass;
    bool isAvailable;
    double price;

public:
    Seat(string seatNumber, string seatClass, double price)
        : seatNumber(seatNumber), seatClass(seatClass),
        price(price), isAvailable(true) {
    }

    // Methods

    bool reserve() {
        if (!isAvailable) {
            cout << "Seat " << seatNumber << " is already reserved.\n";
            return false;
        }
        isAvailable = false;
        cout << "Seat " << seatNumber << " reserved successfully.\n";
        return true;
    }
    bool release() {
        if (isAvailable) {
            cout << "Seat " << seatNumber << " is already available.\n";
            return false;
        }
        isAvailable = true;
        cout << "Seat " << seatNumber << " released successfully.\n";
        return true;
    }

    // Getters 

    double getPrice() const {
        return price;
    }
    // likely needed by Flight/Booking
    string getSeatNumber()  const { return seatNumber; }
    string getSeatClass()   const { return seatClass; }
    bool   getIsAvailable() const { return isAvailable; }
};