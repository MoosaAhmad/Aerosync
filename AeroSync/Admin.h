#ifndef ADMIN_H
#define ADMIN_H

#pragma once
#include "Person.h"
#include<string>
class Admin:public Person
{
    string adminId;
    //int accessLevel;
public:
    /*addFlight()
    removeFlight()
    calcProfitLoss()
    viewAllBookings()
    openHelpPage()*/
    Admin(std::string adminID, std::string name, std::string email, std::string password, bool encrypted = false) :
        Person(name, email, password, encrypted), adminId(adminID) {
    }
    Admin& operator = (const Admin& other) {
        if (this != &other) {
            adminId = other.adminId;
            name = other.name;
            email = other.email;
            passwordHash = other.passwordHash;
        }
        return *this;
    }
    std::string  get_AdminId() const { return adminId; };
};

#endif // ADMIN_H
