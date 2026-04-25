#pragma once
#include<iostream>
#include"Flight.h"
#include"User.h"
#include"Seat.h"
#include"datetime.h"

using namespace std;

class Booking {
protected:
	string bookingId;
	User* user; 
	Flight* flight;
	Seat* seat;
	datetime* bookingDate;
	string status;
public:
	Booking(string Id, User* u, Flight* f, Seat* s, datetime* booking, string stat)
	{
		bookingId = Id;
		user = u;
		flight = f;
		seat = s;
		bookingDate = booking;
		status = stat;
	}
	string getBookingId()
	{
		return bookingId;
	}
	string getstatus()
	{
		return status;
	}
	Flight* getFlight()
	{
		return flight;
	}
	User* getuser()
	{
		return user;
	}
	Seat* getSeat()
	{
		return seat;
	}

	void confirm()
	{
		status = "Confirmed";
		seat->reserve();
		cout << "Dear " << user->getname();
		cout << ",your boking with Id "<<bookingId<<" is confirmed. \n";
		cout << "Seat:" << seat->getSeatNumber() << "reserved"<<endl;
		
	}
	double calcRefund()
	{
		if (status == "Cancelled")
		{
			return seat->getPrice();
		}
		return 0.0;
	}
	void cancel()
	{
		status = "Cancelled";
		seat->release();
		cout << "Dear" << user->getname() << " ,your booking with Id"<<bookingId;
		cout << "is cancelled.\n";
		cout << "Refund: " << calcRefund() << endl;
	}

	// to be used by user class
	void display() const {
		cout << "-------------------------\n";
		cout << "Booking ID   : " << bookingId << "\n";
		cout << "Status       : " << status << "\n";
		cout << "Booking Date : " << bookingDate << "\n";

		if (user)
			cout << "User         : " << user->getName() << " (ID: " << user->getUserId() << ")\n";

		if (flight)
			cout << "Flight       : " << flight->getId()
			<< " | " << flight->getSource()
			<< " -> " << flight->getDestination() << "\n";

		if (seat)
			cout << "Seat         : " << seat->getSeatNumber()
			<< " | Class: " << seat->classToString(seat->getSeatClass())
			<< " | Price: $" << seat->getPrice() << "\n";

		cout << "-------------------------\n";
	}
};