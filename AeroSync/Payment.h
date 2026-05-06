#pragma once
#include <string>
#include <iostream>
#include <stdexcept>
#include "Parser.h"

class Payment {
public:
    enum class PaymentStatus { Paid, Refunded };

protected:
    std::string transactionId;
    double      amount;
    std::string bankName;
    std::string paymentMethod;
    std::string accountNo;
    PaymentStatus status;

public:


    Payment(std::string tId, double amt, std::string bank,
            std::string method, std::string account_no,
            PaymentStatus stat = PaymentStatus::Paid)
        : transactionId(tId),
        amount(amt),
        bankName(bank),
        paymentMethod(method),
        accountNo(account_no),
        status(stat) {
    }

    Payment(const Payment& other)
        : transactionId(other.transactionId),
        amount(other.amount),
        bankName(other.bankName),
        paymentMethod(other.paymentMethod),
        accountNo(other.accountNo),
        status(other.status) {
    }


    std::string getTransactionId() const { return transactionId; }
    double getAmount() const { return amount; }
    std::string getBankName() const { return bankName; }
    std::string getPaymentMethod() const { return paymentMethod; }
    std::string getAccountNo() const { return accountNo; }
    PaymentStatus getStatus() const { return status; }
    std::string getAccountNumber(){return accountNo;}
    char getStatusChar() const {
        return (status == PaymentStatus::Paid) ? 'P' : 'R';
    }

    void updatePaymentStatus(PaymentStatus newStatus) {
        status = newStatus;
    }

    static std::string serialize(const Payment& p) {
        return p.transactionId + "|" +
               std::to_string(p.amount) + "|" +
               p.bankName + "|" +
               p.paymentMethod + "|" +
               p.accountNo + "|" +
               std::string(1, (p.status == PaymentStatus::Paid ? 'P' : 'R'));
    }


    static Payment* deserialize(const std::string& line) {
        auto slices = Parser::slice(line, '|');

        if (slices.size() != 6)
            throw std::runtime_error(
                "6 fields were expected but got " +
                std::to_string(slices.size())
                );

        PaymentStatus stat;

        if (slices[5] == "P")
            stat = PaymentStatus::Paid;
        else if (slices[5] == "R")
            stat = PaymentStatus::Refunded;
        else
            throw std::runtime_error("Invalid payment status: " + slices[5]);

        return new Payment(
            slices[0],
            std::stod(slices[1]),
            slices[2],
            slices[3],
            slices[4],
            stat
            );
    }
};