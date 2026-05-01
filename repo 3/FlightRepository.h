#pragma once
#include<vector>
#include"Flight.h"
#include <fstream>
//........................
// It owns flights data
// responsibilities:
//   .load 
//   .save 
//   .search
//........................
class FlightRepository
{
	vector<Flight*>flights;
	string filename;

public:

	FlightRepository(const string& filename): filename(filename) {}

	void load() {
		ifstream file(filename);
		if (!file.is_open())
			throw runtime_error("Error opening file: " + filename);

		int lineNo = 0;
		string line;
		while (getline(file, line)) {
			lineNo++;
			if (line.empty()) continue;
			try {
				flights.push_back(Flight::deserialize(line));
			}
			catch (const runtime_error& e) {
				throw runtime_error(
					"File: \"" + filename +
					"\", Line " + std::to_string(lineNo) +
					": \"" + line +
					"\" corrupted. Reason: " + e.what()
				);
			}
		}
	}

	void save() const {
		ofstream file(filename);
		if (!file.is_open())
			throw runtime_error("Error opening file: " + filename);

		for (const auto* f : flights) {
			file << Flight::serialize(*f) << "\n";
		}
		file.close();
	}

	~FlightRepository() {
		for (auto f : flights) {
			delete f;
		}
		flights.clear();
	}

	Flight* findFlightById(std::string ID) {
		int sz = flights.size();
		for (int i = 0;i < sz;++i) {
			if (flights[i]->getId() == ID) {
				return flights[i];
			}
		}
		return nullptr;
	}

	vector<Flight*> getFlightsByStatus(Flight::FlightStatus status) const {
		vector<Flight*> res;
		for (auto f : flights)
			if (f->getStatus() == status)
				res.push_back(f);
		return res;
	}

	vector<Flight*> getFlightsByRoute(const string& source, const string& destination) const {
		vector<Flight*> res;
		for (auto f : flights)
			if (f->getSource() == source && f->getDestination() == destination)
				res.push_back(f);
		return res;
	}

	vector<Flight*> getFlightsFromSource(string source) {
		vector<Flight*>res;
		int sz = flights.size();
		for (int i = 0;i < sz;++i) {
			if (flights[i]->getSource() == source) {
				res.push_back(flights[i]);
			}
		}
		return res;
	}

	vector<Flight*> getFlightsToDestination(string destination) {
		vector<Flight*>res;
		int sz = flights.size();
		for (int i = 0;i < sz;++i) {
			if (flights[i]->getDestination() == destination) {
				res.push_back(flights[i]);
			}
		}
		return res;
	}

	vector<Flight*> getFlightsByDate(const date& date) const {
		vector<Flight*> res;
		for (auto f : flights) {
			if (f->getDeparture().get_date() == date) res.push_back(f);
		}
		return res;
	}

	Flight* getFlightById(const string& id) {
		for (auto f : flights) {
			if (f->getId() == id) return f;
		}
		return nullptr;
	}

	vector<Flight*>& share() { return flights; }

	void addFlight(const Flight& flight) {
		flights.push_back(new Flight(flight));
	}

	bool updateFlightStatus(const string& id, Flight::FlightStatus status) {
		Flight* f = findFlightById(id);
		if (!f) return false;

		f->setStatus(status);
		return true;
	}
	//format: FLG-1  FLG-2  FLG-3 ...
	std::string getNextId() const {
		int max = 0;

		for (const auto& f : flights) {
			std::string id = f->getId();

			int num = std::stoi(id.substr(4));

			if (num > max)  max = num;
		}
		return "FLG-" + std::to_string(max + 1);
	}

};
