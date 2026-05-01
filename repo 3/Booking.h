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
	Booking(std::string bookingId, std::string userid, std::string flightid, std::string seatNo, datetime bookingTime, BookingStatus stat)
		:bookingId(bookingId), userId(userid), flightId(flightid), seatNo(seatNo), bookingDate(bookingDate), status(stat) {
	};
	
	std::string getBookingId() { return bookingId; }
	BookingStatus getStatus() { return status; }
	std::string getFlightId() { return flightId; }
	std::string getUserId() { return userId; }
	std::string getSeatNumber() { return seatNo; }
	datetime    getBookingDate() { return bookingDate; }


	/*

//
//void confirm()
//{
//	status = "Confirmed";
//	seat->reserve();
//	cout << "Dear " << User->getname();
//	cout << ",your boking with Id " << bookingId << " is confirmed. \n";
//	cout << "Seat:" << seat->getSeatNumber() << "reserved" << endl;
//
//}

//double calcRefund() {
//	return status == "Cancelled" ? 0.0 : seat->getPrice();
//}

//void cancel()
//{
//	status = "Cancelled";
//	seat->release();
//	cout << "Dear" << User->getname() << " ,your booking with Id" << bookingId;
//	cout << "is cancelled.\n";
//	cout << "Refund: " << calcRefund() << endl;
//}

*/

};
