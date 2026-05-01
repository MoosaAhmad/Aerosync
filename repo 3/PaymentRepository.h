#pragma once
#include"Payment.h"
#include"Vector.h"
#include<vector>
#include<fstream>
#include<string>
class PaymentRepository {
	vector<Payment*>payments;
	std::string filename;
public:
	PaymentRepository(const string& filename) :filename(filename) {}

    void load() {
        ifstream file(filename);
        if (!file.is_open())
            throw runtime_error("error opening file: " + filename);

        string buffer;
        while (getline(file, buffer)) {
            if (buffer.empty()) continue;
            payments.push_back(Payment::deserialize(buffer));
        }
    }

	void save()const {
		std::ofstream file(filename);
		if (!file.is_open())throw std::runtime_error("error opening the file " + filename);
		for (auto r : payments) {
			file << Payment::serialize(*r) << std::endl;
		}
		file.close();
	}

	Payment* getPaymentById(const string& transactionId) {
		for (auto p : payments) {
			if (p->getTransactionId() == transactionId)
				return p;
		}
		return nullptr;
	}

	bool addPayment(Payment& paymnt) {
		for (auto& p : payments) {
			if (p->getTransactionId() == paymnt.getTransactionId())
				return false;
		}
		payments.push_back(new Payment(paymnt));
		return true;
	}

	~PaymentRepository() {
		for (auto p : payments)
			delete p;
	}
};
