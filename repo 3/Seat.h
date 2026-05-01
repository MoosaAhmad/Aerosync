#pragma once
#include<iostream>
class Seat {
public:
    enum class SeatClass{
        Economy,
        Business
    };
private:
    string seatNumber;
    SeatClass seatClass;
    bool isAvailable;
    double price;

public:
    //....................
    // Ctor & Assignment
    //....................
    Seat(string seatNumber, SeatClass seatClass, double price)
        : seatNumber(seatNumber), seatClass(seatClass),
        price(price), isAvailable(true) {
    }
    Seat& operator = (const Seat& other) {
        if (this != &other) {
            seatNumber = other.seatNumber;
            seatClass = other.seatClass;
            isAvailable = other.isAvailable;
            price = other.isAvailable;
        }
        return *this;
    }
    //...........
    // Methods
    //...........
    bool reserve() {
        if (!isAvailable) {
            cout << "Seat " << seatNumber << " is already reserved.\n";
            return false;// already booked
        }
        isAvailable = false;
        cout << "Seat " << seatNumber << " reserved successfully.\n";
        return true;
    }
    bool release() {
        if (isAvailable) {
            cout << "Seat " << seatNumber << " is already available.\n";
            return false; // already reserved
        }
        isAvailable = true;
        cout << "Seat " << seatNumber << " released successfully.\n";
        return true;
    }

    //...........
    // Getters
    //...........
    double getPrice() const {
        return price;
    }

    static string classToString(SeatClass seatclas) {
        switch (seatclas) {
        case SeatClass::Economy: return "Economy";
        case SeatClass::Business: return "Business";
        }
        return "Unknown";
    }

    // likely needed by Flight/Booking
    string   getSeatNumber()  const { return seatNumber; }
    SeatClass getSeatClass()  const { return seatClass; }
    bool    getIsAvailable()  const { return isAvailable; }
};
