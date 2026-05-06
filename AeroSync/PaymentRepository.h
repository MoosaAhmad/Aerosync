#pragma once
#include "Payment.h"
#include <vector>
#include <fstream>
#include <string>
#include <stdexcept>

class PaymentRepository {
    std::vector<Payment*> payments;
    std::string filename;

public:
    PaymentRepository(const std::string& filename) : filename(filename) {}

    void load() {
        std::ifstream file(filename);
        if (!file.is_open())
            throw std::runtime_error("error opening file: " + filename);

        int lineNo = 0;
        std::string buffer;

        while (std::getline(file, buffer)) {
            lineNo++;
            if (buffer.empty()) continue;

            try {
                payments.push_back(Payment::deserialize(buffer));
            }
            catch (const std::runtime_error& e) {
                throw std::runtime_error(
                    "Error: " + filename +
                    " LineNumber: " + std::to_string(lineNo) +
                    " Reason: " + e.what()
                    );
            }
        }
    }

    void save() const {
        std::ofstream file(filename);
        if (!file.is_open())
            throw std::runtime_error("error opening file: " + filename);

        for (auto p : payments) {
            file << Payment::serialize(*p) << std::endl;
        }
    }

    Payment* getPaymentById(const std::string& transactionId) {
        for (auto p : payments) {
            if (p->getTransactionId() == transactionId)
                return p;
        }
        return nullptr;
    }

    bool addPayment(const Payment& payment) {
        if (getPaymentById(payment.getTransactionId()) != nullptr)
            return false;

        payments.push_back(new Payment(payment));
        return true;
    }

    ~PaymentRepository() {
        for (auto p : payments)
            delete p;
    }
};