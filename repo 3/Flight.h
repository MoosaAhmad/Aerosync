#pragma once
#include "DateTime.h"
#include <vector>
#include<string>
#include "Seat.h"
#include"Parser.h"

class Flight {
public:
    enum class FlightStatus {
        Scheduled,
        Delayed,
        Cancelled,
        Departed,
        Arrived
    };
private:
    std::string flightId;
    string source;
    string destination;
    datetime departure;
    datetime arrival;
    FlightStatus status;

    int totalEconomySeats;
    int totalBusinessSeats;

    double economyPrice;
    double businessPrice;

    vector<Seat> seats;
public:

    static string serialize(const Flight& f) {
        std::string line;

        line += f.flightId + '|';
        line += f.source + '|';
        line += f.destination + '|';

        line += datetime::toString(f.departure) + '|';
        line += datetime::toString(f.arrival) + '|';

        char tmp = ' ';
        switch (f.status) {
        case FlightStatus::Scheduled: tmp = 'S'; break;
        case FlightStatus::Delayed:   tmp = 'L'; break;
        case FlightStatus::Cancelled: tmp = 'C'; break;
        case FlightStatus::Departed:  tmp = 'D'; break;
        case FlightStatus::Arrived:   tmp = 'A'; break;
        }

        line.push_back(tmp);
        line.push_back('|');

        line += std::to_string(f.totalEconomySeats) + '|';
        line += std::to_string(f.totalBusinessSeats) + '|';
        line += std::to_string(f.economyPrice) + '|';
        line += std::to_string(f.businessPrice) + '|';

        // seats contain economy first later business seats
        for (int i = 0; i < f.totalEconomySeats + f.totalBusinessSeats; i++) {
            line.push_back(f.seats[i].getIsAvailable() ? 'A' : 'B');
        }
        
        return line;
    }


    static Flight* deserialize(const string& line) {
        auto slices = Parser::slice(line, '|');

        if (slices.size() != 11) {
            throw std::runtime_error("11 fields were expected but got " + 
                  std::to_string(slices.size())+
                " fields");
        }
        FlightStatus status = FlightStatus::Scheduled;


        if (slices[5].empty())
            throw runtime_error("Empty flight status field");

 
        char tmp = slices[5][0];
        switch (tmp) {
        case 'S': status = FlightStatus::Scheduled; break;
        case 'L': status = FlightStatus::Delayed;   break;
        case 'C': status = FlightStatus::Cancelled; break;
        case 'D': status = FlightStatus::Departed;  break;
        case 'A': status = FlightStatus::Arrived;   break;
        default: throw std::runtime_error("invalid FlightStatus \'" + tmp + '\'');
        }

    
        int economySeats,businessSeats; 
        double economyPrice, businessPrice; 
        try {
            economySeats = stoi(slices[6]);
            businessSeats = stoi(slices[7]);
            economyPrice = stod(slices[8]);
            businessPrice = stod(slices[9]);
        }
        catch (const invalid_argument& e) {
            throw runtime_error("Invalid numeric data in record: " + string(e.what()));
        }

        datetime d1, d2;
        try {
            d1 = datetime::fromString(slices[3]);
            d2 = datetime::fromString(slices[4]);
        }
        catch (const invalid_argument& e) {
            throw runtime_error( string(e.what()));
        }

        Flight* flight = new Flight(
            slices[0], slices[1], slices[2],
            d1, d2,
            economySeats,
            businessSeats,
            economyPrice,
            businessPrice,
            status
        );

        string seatString = slices[10];

        int totalSeats = economySeats + businessSeats;
        if (seatString.size() != totalSeats) {
            throw std::runtime_error(
            "number of seats and number of Seat Status are inconsistent"
            );
        }
        for (int i = 0; i < totalSeats; i++) {
            if (seatString[i] == 'B') {
                flight->seats[i].reserve();
            }
        }

        return flight;
    }

    //...........
    // Assignment
    //...........
    Flight(std::string flightId, string source, string destination, datetime departure, datetime arrival,
        int totalEconomySeats, int totalBusinessSeats, double economyPrice, double businessPrice,
        FlightStatus status = FlightStatus::Scheduled)
        : flightId(flightId),
        source(source),
        destination(destination),
        departure(departure),
        arrival(arrival),
        status(status),
        totalEconomySeats(totalEconomySeats),
        totalBusinessSeats(totalBusinessSeats),
        economyPrice(economyPrice),
        businessPrice(businessPrice)
    {
        generateSeats();
    }

    Flight(const Flight& other)
        : flightId(other.flightId),
        source(other.source),
        destination(other.destination),
        departure(other.departure),
        arrival(other.arrival),
        status(other.status),
        seats(other.seats),
        totalEconomySeats(other.totalEconomySeats),
        totalBusinessSeats(other.totalBusinessSeats),
        economyPrice(other.economyPrice),
        businessPrice(other.businessPrice)
    {
    }

    Flight& operator = (const Flight& other) {
        if (this != &other) {
            flightId = other.flightId;
            source = other.source;
            destination = other.destination;
            departure = other.departure;
            arrival = other.arrival;
            status = other.status;
            seats = other.seats;

            economyPrice = other.economyPrice;
            businessPrice = other.businessPrice;

            totalEconomySeats = other.totalEconomySeats;
            totalBusinessSeats = other.totalBusinessSeats;
        }
        return *this;
    }

    bool bookSeat(string seatNo) {
        if (status != FlightStatus::Scheduled)return false; // booking is not opened

        int id = -1;
        int sz = seats.size();
        for (int i = 0;i < sz;++i) {
            if (seatNo == seats[i].getSeatNumber()) {
                id = i;break;
            }
        }
        if (id == -1) return false; // no such seat exist

        return  seats[id].reserve();
    }

    vector<Seat*> getAvailableSeats() {
        vector<Seat*>res;

        int sz = seats.size();
        for (int i = 0;i < sz;++i) {
            if (seats[i].getIsAvailable()) {
                res.push_back(&seats[i]);
            }
        }
        return res;
    }

    vector<Seat*> getReservedSeats() {
        vector<Seat*>res;

        int sz = seats.size();
        for (int i = 0;i < sz;++i) {
            if (!seats[i].getIsAvailable()) {
                res.push_back(&seats[i]);
            }
        }
        return res;
    }



    void generateSeats() {
        for (int i = 0;i < totalEconomySeats;i++) {
            seats.push_back(Seat("E" + to_string(i + 1), Seat::SeatClass::Economy, economyPrice));
        }
        for (int i = 0;i < totalBusinessSeats;i++) {
            seats.push_back(Seat("B" + to_string(i + 1), Seat::SeatClass::Business, businessPrice));
        }
    }
    //...........
    //  Getters
    //...........

    std::string getId()      const { return flightId; }
    string getSource()       const { return source; }
    string getDestination()  const { return destination; }
    datetime getDeparture()  const { return departure; }
    datetime getArrival()    const { return arrival; }
    FlightStatus getStatus() const { return status; }

    void setStatus(FlightStatus s) { status = s; }
    //..............................
    // [enum]->[string]
    //    can be used in UI
    //..............................
    static string statusToString(FlightStatus s) {
        switch (s) {
        case FlightStatus::Scheduled: return "Scheduled";
        case FlightStatus::Delayed:   return "Delayed";
        case FlightStatus::Cancelled: return "Cancelled";
        case FlightStatus::Departed:  return "Departed";
        case FlightStatus::Arrived:   return "Arrived";
        }
        return "Unknown";
    }

};
