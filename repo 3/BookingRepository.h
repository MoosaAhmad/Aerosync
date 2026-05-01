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

        int lineNo = 0;
        string buffer;
        while (getline(file, buffer)) {
            lineNo++;
            if (buffer.empty()) continue;
            try {
                bookings.push_back(deserialize(buffer));
            }
            catch (const runtime_error& e) {
                throw runtime_error(
                    "File: \"" + filename +
                    "\", Line " + std::to_string(lineNo) +
                    ": \"" + buffer +
                    "\" corrupted. Reason: " + e.what()
                );
            }

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
        for (auto &b : bookings) if (b->getFlightId() == FlightId && b->getSeatNumber() == seatNo && b->getStatus()== Booking::BookingStatus::Booked) return b;
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


    //format: BKN-1  BKN-2  BKN-3 ...
    std::string getNextId() const {
        int max = 0;

        for (const auto& f : bookings) {
            std::string id = f->getBookingId();  

            int num = std::stoi(id.substr(4));

            if (num > max)  max = num;
        }
        return "BKN-" + std::to_string(max + 1);
    }

private:
    static std::string serialize(Booking& b) {
        string line;
        line.reserve(75);

        line += b.getBookingId();  line.push_back('|');
        line += b.getUserId(); 	   line.push_back('|');
        line += b.getFlightId();   line.push_back('|');
        line += b.getSeatNumber(); line.push_back('|');
        line += datetime::toString(b.getBookingDate()); line.push_back('|');
        Booking::BookingStatus st = b.getStatus();
        line.push_back(st == Booking::BookingStatus::Booked ? 'B' : 'C');
        return line;
    }

    static Booking* deserialize(const string& line) {
        auto slices = Parser::slice(line, '|');

        if (slices.size() != 6) {
            throw std::runtime_error("6 fields were expected but got " +
                std::to_string(slices.size()) +
                " fields");

        }
        if (slices[5].empty())
            throw runtime_error("Empty Booking status field");

        char st = slices[5][0];
        if(st!='B' && st!='C') 
            throw runtime_error("invalid Booking status \'" + st+'\'');

        Booking::BookingStatus bs = (st == 'B') ? Booking::BookingStatus::Booked : Booking::BookingStatus::Cancelled;


        datetime d1;
        try {
            d1 = datetime::fromString(slices[4]);
        }
        catch (const invalid_argument& e) {
            throw runtime_error(string(e.what()));
        }

        return new Booking(slices[0], slices[1], slices[2],
            slices[3], d1, bs
        );
    }
};