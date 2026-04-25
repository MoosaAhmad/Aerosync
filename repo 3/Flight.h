#pragma once
#include "DateTime.h"
#include "Vector.h";
#include"Seat.h"
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
	int flightId;
	string source;
	string destination;
	datetime departure;
	datetime arrival;
    FlightStatus status;
	Vector<Seat> seats;
public:
    //...........
    // Assignment
    //...........
    Flight& operator = (const Flight& other) {
        if (this != &other) {
            flightId = other.flightId;
            source = other.source;
            destination = other.destination;
            departure = other.departure;
            arrival = other.arrival;
            status = other.status;
            seats = other.seats;
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

    Vector<Seat*> getAvailableSeats() {
        Vector<Seat*>res;

        int sz = seats.size();
        for (int i = 0;i < sz;++i) {
            if (seats[i].getIsAvailable()) {
                res.push_back(&seats[i]);
            }
        }
        return res;
    }
    Vector<Seat*> getReservedSeats() {
        Vector<Seat*>res;

        int sz = seats.size();
        for (int i = 0;i < sz;++i) {
            if (!seats[i].getIsAvailable()) {
                res.push_back(&seats[i]);
            }
        }
        return res;
    }
   
    //...........
    //  Getters
    //...........

    int getId()              const { return flightId; }
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
    
    // to be used by user class
    void displayStatus()
    {
        string temp = statusToString(status);
        cout << "Status : " << temp << endl;
    }
};
