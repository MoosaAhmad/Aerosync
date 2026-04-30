#pragma once
#include <vector>
#include <fstream>
#include <string>
#include "Parser.h"
#include "Booking.h"
/*
Add validation Function in datetimme and use it in every parser
*/
class BookingRepository {
    vector<Booking*> bookings;
    std::string filename;
public:

    BookingRepository(string filename) : filename(filename) {}

    void load() {
        ifstream file(filename);
        if (!file.is_open())
            throw runtime_error("error opening file: " + filename);

        string buffer;
        while (getline(file, buffer)) {
            if (buffer.empty()) continue;
            bookings.push_back(deserialize(buffer));
        }
    }

    Booking* getBookingById(const std::string& Id) const {
        int sz = bookings.size();
        for (int i = 0;i < sz;i++) {
            if (bookings[i]->getBookingId() == Id)
                return bookings[i];
        }
        return nullptr;
    }

    vector<Booking*>getFlightBooking(const string& flightId) {
        vector<Booking*>res;
        for (auto b : bookings) if (b->getFlightId() == flightId) res.push_back(b);
        return res;
    }

    vector<Booking*>getUserHistory(const string& userId) {
        vector<Booking*>res;
        for (auto b : bookings) if (b->getUserId() == userId) res.push_back(b);
        return res;
    }

    Booking* findBookedSeat(const string& FlightId,const string& seatNo) {
        for (auto &b : bookings) if (b->getFlightId() == FlightId && b->getSeatNumber() == seatNo && b->getStatus()=="booked") return b;
        return nullptr;
    }

    void save() {
        ofstream file(filename);
        if (!file.is_open())
            throw runtime_error("error opening file");

        for (auto& b : bookings)
            file << serialize(*b) << endl;
    }

    void addBooking(const Booking& b) {
        bookings.push_back(new Booking(b));
    }

    ~BookingRepository() {
        for (auto& b : bookings) delete b;
    }

    vector<Booking*>share(){ return bookings; }

private:
    static std::string serialize(Booking& b) {
        string line;
        line.reserve(75);

        line += b.getBookingId();  line.push_back('|');
        line += b.getUserId(); 	   line.push_back('|');
        line += b.getFlightId();   line.push_back('|');
        line += b.getSeatNumber(); line.push_back('|');
        line += datetime::toString(b.getBookingDate()); line.push_back('|');
        line += b.getStatus();

        return line;
    }

    static Booking* deserialize(const string& line) {
        auto slices = Parser::slice(line, '|');

        if (slices.size() != 6) throw runtime_error("invalid format: " + line);

        return new Booking(slices[0], slices[1], slices[2],
            slices[3], datetime::fromString(slices[4]), slices[5]
        );
    }
};