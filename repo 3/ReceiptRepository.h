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
		
		int lineNo = 0;
		std::string buffer;
		while (getline(file, buffer)) {
			lineNo++;
			if (buffer.empty())continue;
			try {
				receipts.push_back(deserialize(buffer));
			}
			catch (const std::runtime_error& e) {
				throw std::runtime_error(
					"File: \"" + filename +
					"\", Line " + std::to_string(lineNo) +
					": \"" + buffer +
					"\" corrupted. Reason: " + e.what()
				);
			}
		}
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
	//format: RCP-1  RCP-2  RCP-3 ...
	std::string getNextId() const {
		int max = 0;

		for (const auto& r : receipts) {
			std::string id = r->getReceiptId();

			int num = std::stoi(id.substr(4));

			if (num > max)  max = num;
		}
		return "RCP-" + std::to_string(max + 1);
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
		if (slices.size() != 3) {
			throw std::runtime_error("3 fields were expected but got " +
				std::to_string(slices.size()) +
				" fields");
		}
		return new Receipt(slices[0], slices[1], slices[2]);
	}
};