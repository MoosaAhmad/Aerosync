#pragma once
#include<iostream>
#include"Flight.h"
#include"User.h"
using namespace std;

class Booking {
protected:
	string bookingId;
	user* User; 
	Flight* flight;
	Seat* seat;
	datetime* bookingDate;
	string status;
public:
	Booking(string Id, user* u, Flight* f, Seat* s, datetime* booking, string stat)
	{
		bookingId = Id;
		User = u;
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
	user* getuser()
	{
		return User;
	}
	Seat* getSeat()
	{
		return seat;
	}

	void confirm()
	{
		status = "Confirmed";
		seat->reserve();
		cout << "Dear " << User->getname();
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
		cout << "Dear" << User->getname() << " ,your booking with Id"<<bookingId;
		cout << "is cancelled.\n";
		cout << "Refund: " << calcRefund() << endl;
	}
};