#pragma once
#include "DateTime.h"

class Flight;
class user;
class Seat;


class Booking {
protected:
	std::string bookingId;
	std::string userId;
	std::string flightId;
	std::string seatNo;
	datetime bookingDate;
	std::string status;
public:
	Booking(std::string bookingId, std::string userid, std::string flightid, std::string seatNo, datetime bookingTime, std::string stat);
	
	std::string getBookingId();
	std::string getStatus();
	std::string getFlightId();
	std::string getUserId();
	std::string getSeatNumber();
	datetime    getBookingDate();


	/*void confirm();
	double calcRefund();
	void cancel();*/
};
