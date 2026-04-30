#pragma once
#include "Person.h"
#include<vector>
#include "Booking.h"

class user :public Person
{
	int userId;
	vector<Booking*>history; // just 
public:
	//.........
	// Ctor
	//.........
	user(int userId, string name, string email, string password, bool encrypted = false) :
		Person(name, email, password, encrypted), userId(userId) {
	}
	user& operator = (const user& other) {
		if (this != &other) {
			userId = other.userId;
			name = other.name;
			email = other.email;
			passwordHash = other.passwordHash;
			history = other.history;
		}
		return *this;
	}
	//.........
	// Getters
	//.........
	int get_userId() const { return userId; };

	// used while re-establishing links while loading and and establishing
	void addBooking(Booking* ptr) {
		history.push_back(ptr);
	}


	//	void authenticate() override {}
};
