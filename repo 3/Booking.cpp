#include "Booking.h"
#include <string>

Booking::Booking(std::string bookingId, std::string userid, std::string flightid, std::string seatNo, datetime bookingDate, Booking::BookingStatus stat)
	:bookingId(bookingId), userId(userid), flightId(flightid), seatNo(seatNo),bookingDate(bookingDate),status(stat) {
};
//..........
// Getters
//..........

Booking::BookingStatus Booking::getStatus()     { return status; }
std::string  Booking::getFlightId()   { return flightId; }
std::string  Booking::getUserId()     { return userId;}
std::string  Booking::getSeatNumber() { return seatNo;}
std::string  Booking::getBookingId()  { return bookingId; }
datetime     Booking::getBookingDate(){ return bookingDate; }

/*

//
//void Booking::confirm()
//{
//	status = "Confirmed";
//	seat->reserve();
//	cout << "Dear " << User->getname();
//	cout << ",your boking with Id " << bookingId << " is confirmed. \n";
//	cout << "Seat:" << seat->getSeatNumber() << "reserved" << endl;
//
//}

//double Booking::calcRefund() {
//	return status == "Cancelled" ? 0.0 : seat->getPrice();
//}

//void Booking::cancel()
//{
//	status = "Cancelled";
//	seat->release();
//	cout << "Dear" << User->getname() << " ,your booking with Id" << bookingId;
//	cout << "is cancelled.\n";
//	cout << "Refund: " << calcRefund() << endl;
//}

*/
