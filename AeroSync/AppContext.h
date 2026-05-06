#ifndef APPCONTEXT_H
#define APPCONTEXT_H

#pragma once
#include "FlightRepository.h"
#include "BookingRepository.h"
#include "PaymentRepository.h"
#include "ReceiptRepository.h"
#include "userRepository.h"

// Shared data context — owns all repositories
struct AppContext {

    BookingRepository bookingRepo;
    FlightRepository  flightRepo;
    PaymentRepository paymentRepo;
    ReceiptRepository receiptRepo;
    UserRepository    userRepo;

    explicit AppContext(const std::string& dir = "")
        :bookingRepo(dir + "bookings.txt"),
        flightRepo(dir + "flights.txt"),
        paymentRepo(dir + "Payments.txt"),
        receiptRepo(dir + "receipts.txt"),
        userRepo(dir + "users.txt",dir + "admins.txt")
    {}

    void loadAll() {
        flightRepo.load();
        bookingRepo.load();
        paymentRepo.load();
        userRepo.loadUser();
        userRepo.loadAdmins();
        try { receiptRepo.load(); }
        catch (...) {} // receipts.txt may not exist yet
    }
};


#endif // APPCONTEXT_H
