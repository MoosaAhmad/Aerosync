#pragma once
#include "DateTime.h"


class Booking {
public:
	enum class BookingStatus { Booked, Cancelled };
protected:
	std::string bookingId;
	std::string userId;
	std::string flightId;
	std::string seatNo;
	datetime bookingDate;
	BookingStatus status;
public:
	Booking(std::string bookingId, std::string userid, std::string flightid, std::string seatNo, datetime bookingTime, BookingStatus stat);
	
	std::string getBookingId();
	BookingStatus getStatus();
	std::string getFlightId();
	std::string getUserId();
	std::string getSeatNumber();
	datetime    getBookingDate();


	/*void confirm();
	double calcRefund();
	void cancel();*/
};
