#pragma once
#include"Receipt.h"
#include"Parser.h"
#include<vector>
#include<fstream>

class ReceiptRepository {
	std::string filename;
	std::vector<Receipt*>receipts;
public:
	ReceiptRepository(const std::string& filename) :filename(filename) {}
	~ReceiptRepository() {
		for (auto r : receipts) delete r;
	}

	void load() {
		std::ifstream file(filename);
		if (!file.is_open())throw std::runtime_error("error opening the file " + filename);
		std::string buffer;
		while (getline(file, buffer)) {
			if (buffer.empty())continue;
			receipts.push_back(deserialize(buffer));
		}
		file.close();
	}
	void save()const{
		std::ofstream file(filename);
		if (!file.is_open())throw std::runtime_error("error opening the file " + filename);
		for (auto r : receipts) {
			file << serialize(*r) << std::endl;
		}
		file.close();
	}

	void addReceipt(const Receipt& r){
			receipts.push_back(new Receipt(r));
	}

	Receipt* searchByReceiptId(const std::string& id)
	{
		for (auto r : receipts){
			if (r->getReceiptId() == id)
				return r;
		}
		return nullptr;
	}
	Receipt* searchByPaymentId(const std::string& paymentId)
	{
		for (auto r : receipts)
		{
			if (r->getPaymentId() == paymentId)
				return r;
		}
		return nullptr;
	}
	Receipt* searchByBookingId(const std::string& bookingId){
		for (auto r : receipts)	{
			if (r->getBookingId() == bookingId)
				return r;
		}
		return nullptr;
	}
private:
	std::string serialize(const Receipt&r) const {
		std::string line;
		line += r.getReceiptId();
		line.push_back('|');
		line += r.getBookingId();
		line.push_back('|');
		line += r.getPaymentId();
		return line;
	}
	Receipt* deserialize(const std::string line)const {
		auto slices = Parser::slice(line, '|');
		if (slices.size() != 3)throw std::runtime_error("invalid format: " + line);
		return new Receipt(slices[0], slices[1], slices[2]);
	}
};