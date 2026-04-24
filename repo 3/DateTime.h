#pragma once

using namespace std;
class duration {
	bool negative;
	int minutes;
	int hours;
	int days;
public:
	//......................
	//  Ctors & Assignment 
	//......................
	duration() :minutes(0), hours(0), days(0), negative(false) {};
	duration(int d, int h, int m, bool neg = false) {
		if (d < 0 || h < 0 || m < 0)
			throw invalid_argument("invalid duration");

		negative = neg;

		int total = d * 1440 + h * 60 + m;

		days = total / 1440;
		total %= 1440;

		hours = total / 60;
		minutes = total % 60;
	}
	duration(const duration& other)
		: days(other.days), hours(other.hours), minutes(other.minutes), negative(other.negative) {
	}
	duration& operator = (const duration& other) {
		if (this != &other) {
			days = other.days;
			hours = other.hours;
			minutes = other.minutes;
			negative = other.negative;
		}
		return *this;
	}

	static duration fromDays(int DAYS) {
		duration tmp;
		tmp.negative = DAYS < 0;
		tmp.days = abs(DAYS);
		return tmp;
	}
	static duration fromHours(int hrs) {
		duration tmp;
		tmp.negative = hrs < 0;
		hrs = abs(hrs);

		tmp.days = hrs / 24;
		tmp.hours = hrs % 24;
		tmp.minutes = 0;

		return tmp;
	}
	static duration fromMinutes(int mints) {
		duration tmp;
		tmp.negative = mints < 0;
		mints = abs(mints);

		tmp.days = mints / 1440;
		mints %= 1440;

		tmp.hours = mints / 60;
		tmp.minutes = mints % 60;

		return tmp;
	}

	int totalMinutes() const {
		int total = days * 1440 + hours * 60 + minutes;
		return negative ? -total : total;
	}
	int totalHours() const {
		int total = days * 24 + hours + minutes / 60;
		return negative ? -total : total;
	}
	int totalDays() const {
		int total = days + hours / 24;
		return negative ? -total : total;
	}

	duration operator + (const duration& other) const {
		return duration::fromMinutes(this->totalMinutes() + other.totalMinutes());
	}
	duration operator - (const duration& other) const {
		return duration::fromMinutes(this->totalMinutes() - other.totalMinutes());
	}
	duration& operator += (const duration& other) {
		*this = *this + other;
		return *this;
	}
	duration& operator -= (const duration& other) {
		*this = *this - other;
		return *this;
	}
	duration operator - () const {
		duration tmp = *this;
		tmp.negative = !tmp.negative;
		return tmp;
	}


	string toString() const {
		string tmp = negative ? "-" : "";

		return tmp + to_string(days) + "d " + to_string(hours) + "h " + to_string(minutes) + "m";
	}
};



class date {
	int day;
	int month;
	int year;

	static bool isLeap(int year) {
		return (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);
	}
	static bool isValid_day(int Year, int Month, int Day) {
		if (Day < 1) return false;
		return Day <= ttlDaysInMonth(Month, Year);
	}
	static int ttlDaysInYear(int Year) {
		return isLeap(Year) ? 366 : 365;
	}
	static int ttlDaysInMonth(int m, int y) {
		switch (m) {
		case 1:case 3:case 5:case 7:case 8:case 10:case 12:
			return 31;
		case 2:
			return (isLeap(y) ? 29 : 28);
		case 4:case 6:case 9:case 11:
			return 30;
		default:
			throw runtime_error("invalid month");
		}

	}

	//......................
	//  Low Level Ctor 
	//......................
	date(int DaysbacktoDefault) {
		year = 1900;
		month = 1;
		day = 1;

		while (DaysbacktoDefault >= ttlDaysInYear(year)) {
			DaysbacktoDefault -= ttlDaysInYear(year);
			year++;
		}
		while (DaysbacktoDefault >= ttlDaysInMonth(month, year)) {
			DaysbacktoDefault -= ttlDaysInMonth(month, year);
			month++;
		}
		day += DaysbacktoDefault;
	}
public:
	//......................
	//  Ctors & Assignment 
	//......................

	date() :day(1), month(1), year(1900) {}

	date(int Day, int Month, int Year) {
		if (Month < 1 || Month>12)
			throw runtime_error("invalid month");
		if (Year < 0)
			throw runtime_error("invalid year");
		if (!isValid_day(Year, Month, Day))
			throw runtime_error("invalid day");

		day = Day;
		month = Month;
		year = Year;
	}
	date(const date& other) :day(other.day), month(other.month), year(other.year) {}

	date& operator = (const date& other) {
		if (this != &other) {
			day = other.day;month = other.month;year = other.year;
		}
		return *this;
	}
	//...............
	//  Helper
	//...............
	int daysBackToDefault() const {
		int res = 0;

		for (int y = 1900; y < year; ++y) {
			res += ttlDaysInYear(y);
		}

		for (int m = 1; m < month; ++m) {
			res += ttlDaysInMonth(m, year);
		}

		res += (day - 1);
		return res;
	}
	//...............
	//  comparators
	//...............

	bool operator <  (const date& other) const {
		if (year != other.year)  return year < other.year;
		if (month != other.month) return month < other.month;
		return day < other.day;
	}

	bool operator >(const date& o) const { return o < *this; }

	bool operator>=(const date& o) const { return !(*this < o); }

	bool operator<=(const date& o) const { return !(o < *this); }

	bool operator == (const date& other) const {
		return (day == other.day) && (month == other.month) && (year == other.year);
	}

	bool operator != (const date& other) const {
		return (day != other.day) || (month != other.month) || (year != other.year);
	}

	//...............
	//  Getters
	//...............
	int get_year()  const { return year; }
	int get_month() const { return month; }
	int get_day()   const { return day; }

	//...............
	//  Getters
	//...............
	void set_year(int y) {
		if (y < 0)
			throw runtime_error("invalid year");
		year = y;
	}
	void set_month(int Month) {
		if (Month < 1 || Month>12)
			throw runtime_error("invalid month");
		month = Month;
	}
	void set_day(int Day) {
		if (!isValid_day(year, month, Day))
			throw runtime_error("invalid day");
		day = Day;
	}

	//...............
	//  Utility
	//...............

	//................................
	//    Serialize & Deserialize    :
	//    [format]: DD:MM::YYYY      :
	//................................

	static std::string to_string(date d) {
		std::string str;
		if (d.day < 10)str += '0';
		str += std::to_string(d.day) + '-';
		if (d.month < 10)str += '0';
		str += std::to_string(d.month) + '-';
		str += std::to_string(d.year);
		return str;
	}

	duration operator - (const date& other) const {
		int diff = daysBackToDefault() - other.daysBackToDefault();
		return duration::fromDays(diff);
	}
	date operator + (const duration& d) const {
		return date(daysBackToDefault() + d.totalDays());
	}

	static duration getDuration(const date& d1, const date& d2) {
		return duration::fromDays(d1.daysBackToDefault() - d2.daysBackToDefault());
	}
	date operator - (const duration& d) const {
		return date(daysBackToDefault() - d.totalDays());
	}

	static date fromString(const string& Date) {
		if (Date.length() != 10)
			throw runtime_error("invalid date format");

		if (Date[2] != '-' || Date[5] != '-')
			throw runtime_error("invalid date format");


		for (int i = 0; i < 2; i++)
			if (Date[i] < '0' || Date[i] > '9')
				throw runtime_error("invalid day");

		for (int i = 3; i < 5; i++)
			if (Date[i] < '0' || Date[i] > '9')
				throw runtime_error("invalid month");

		for (int i = 6; i < 10; i++)
			if (Date[i] < '0' || Date[i] > '9')
				throw runtime_error("invalid year");

		// after validation constructing date
		int d = stoi(Date.substr(0, 2));
		int m = stoi(Date.substr(3, 2));
		int y = stoi(Date.substr(6, 4));

		return date(d, m, y);
	}

	string dayOfWeek() const {
		/*
		used an algorithm: Zeller Congruence
		[source: gpt]
		*/
		int d = day;
		int m = month;
		int y = year;

		// Adjust months: March = 3, ..., January = 13, February = 14 (previous year)
		if (m < 3) {
			m += 12;
			y--;
		}

		int K = y % 100;     // Year of the century
		int J = y / 100;     // Zero-based century

		int h = (d + (13 * (m + 1)) / 5 + K + (K / 4) + (J / 4) + (5 * J)) % 7;

		// Mapping result to day
		string days[] = {
			"Saturday", "Sunday", "Monday",
			"Tuesday", "Wednesday", "Thursday", "Friday"
		};

		return days[h];
	}

	date operator + (int Days) const {
		return date(daysBackToDefault() + Days);
	}
	date operator - (int Days) const {
		return date(daysBackToDefault() - Days);
	}



	static date now() {
		//source gpt
		time_t t = time(nullptr);

		tm current;

		localtime_s(&current, &t);

		return date(
			current.tm_mday,
			current.tm_mon + 1,
			current.tm_year + 1900
		);
	}
};

class Time {
	int h;
	int m;
public:
	//...................
	// Ctor & Assignmnt
	//...................
	Time() :h(0), m(0) {};
	Time(int H, int M) {
		if (H < 0 || H>23) throw invalid_argument("invalid hours");
		if (M < 0 || M>59) throw invalid_argument("invalid minutes");
		this->h = H;
		this->m = M;
	}
	Time(const Time& other) :h(other.h), m(other.m) {};

	Time& operator = (const Time& other) {
		if (&other != this) {
			h = other.h;
			m = other.m;
		}
		return *this;
	}
	//..............
	// Comparators
	//..............
	bool operator >  (const Time& other)  const {
		if (h != other.h) return h > other.h;
		return m > other.m;
	}
	bool operator <= (const Time& other) const { return !(*this > other); }
	bool operator <  (const Time& other) const { return   other > *this; }
	bool operator >= (const Time& other) const { return !(*this < other); }
	bool operator == (const Time& other) const { return h == other.h && m == other.m; }
	bool operator != (const Time& other) const { return h != other.h || m != other.m; }

	//............
	// Setters
	//............
	void set_hours(int H) {
		if (H < 0 || H>23) throw invalid_argument("invalid hours");
		h = H;
	}
	void set_minutes(int M) {
		if (M < 0 || M>59) throw invalid_argument("invalid minutes");
		m = M;
	}
	//..........
	// Getters
	//..........
	int get_hours()   const { return h; }
	int get_minutes() const { return m; }
	//...........................
	// Serialize & Deserialize  :
	//   [format]: HH:MM        :
	//...........................
	static string toString(const Time& t) {
		string str = "";
		if (t.h < 10) str += '0';
		str += std::to_string(t.h);
		str += ':';
		if (t.m < 10) str += '0';
		str += std::to_string(t.m);
		return str;
	}
	static Time fromString(const string& str) {
		if (str.length() != 5)
			throw invalid_argument("invalid time format");
		if (str[2] != ':')
			throw invalid_argument("invalid time format");
		for (int i = 0;i < 2;i++)
			if (str[i] < '0' || str[i]>'9')
				throw invalid_argument("invalid time format");

		int H = stoi(str.substr(0, 2));
		int M = stoi(str.substr(3, 2));
		return Time(H, M);

	}

	static Time fromMinutes(int totalMinutes) {
		totalMinutes %= 1440;
		if (totalMinutes < 0)totalMinutes += 1440;
		return Time(totalMinutes / 60, totalMinutes % 60);
	}

	Time operator + (int minutes) const {
		return fromMinutes(h * 60 + m + minutes);
	}
	Time operator - (int minutes) const {
		return fromMinutes(h * 60 + m - minutes);
	}
	Time operator + (const duration& d) const {
		return Time::fromMinutes(h * 60 + m + d.totalMinutes());
	}
	duration operator - (const Time& other) const {
		int diff = (h * 60 + m) - (other.h * 60 + other.m);
		return duration::fromMinutes(diff);
	}
	string to12Hour() const {
		bool pm = h >= 12;
		string tmp = "";
		if (h % 12 == 0) {
			tmp += "12";
		}
		else if (h % 12 < 10) {
			tmp.push_back('0');
			tmp += (h % 12) + '0';
		}
		else {
			tmp += to_string(h % 12);
		}
		tmp.push_back(':');
		if (m < 10)tmp.push_back('0');
		tmp += to_string(m);
		tmp += (pm ? " PM" : " AM");
		return tmp;
	}


	static Time now() {
		// source gpt
#include <ctime>
		time_t t = time(nullptr);

		tm current;
		localtime_s(&current, &t);

		return Time(current.tm_hour, current.tm_min);
	}

	//static bool isValid(int h, int m);
};

class datetime {
	date d;
	Time t;
public:
	//...................
	// Ctor & Assignmnt
	//...................

	datetime() {};
	datetime(int hr, int min, int d, int mon, int yr) :t(hr, min), d(d, mon, yr) {};
	datetime(const datetime& other) :d(other.d), t(other.t) {};
	datetime& operator = (const datetime& other) {
		if (this != &other) {
			d = other.d;
			t = other.t;
		}
		return *this;
	}

	//...................
	// Comparators
	//...................
	bool operator == (const datetime& other) const {
		return d == other.d && t == other.t;
	}
	bool operator != (const datetime& other) const {
		return d != other.d || t != other.t;
	}
	bool operator > (const datetime& other) const {
		if (d != other.d) return d > other.d;
		return t > other.t;
	}
	bool operator <= (const datetime& other) const {
		return !(*this > other);
	}
	bool operator < (const datetime& other) const {
		return other > *this;
	}
	bool operator >= (const datetime& other) const {
		return !(*this < other);
	}
	//............
	// Getters
	//............
	int get_hours()   const { return t.get_hours(); }
	int get_minutes() const { return t.get_minutes(); }
	int get_year()    const { return d.get_year(); }
	int get_month()   const { return d.get_month(); }
	int get_day()     const { return d.get_day(); }
	//............
	// Setters
	//............
	void set_hours(int H) { t.set_hours(H); }
	void set_minutes(int M) { t.set_minutes(M); }
	void set_year(int y) { d.set_year(y); }
	void set_month(int m) { d.set_month(m); }
	void set_day(int Day) { d.set_day(Day); }

	//...................
	// Utility
	//...................

	//................................
	//    Serialize & Deserialize
	// [format]: DD-MM-YYYY HH:MM
	//................................

	static string toString(const datetime& dt) {
		return date::to_string(dt.d) + " " + Time::toString(dt.t);
	}
	static datetime fromString(const string& str) {
		if (str.length() != 16 || str[10] != ' ')
			throw invalid_argument("invalid datetime format");

		string dpart = str.substr(0, 10);
		string tpart = str.substr(11, 5);

		date nd = date::fromString(dpart);
		Time nt = Time::fromString(tpart);

		return datetime(nt.get_hours(), nt.get_minutes(), nd.get_day(), nd.get_month(), nd.get_year());
	}



	string dayOfWeek() const {
		return d.dayOfWeek();
	}

	static datetime now() {
		// source gpt
		time_t t_now = time(nullptr);

		tm current;
		localtime_s(&current, &t_now);

		return datetime(current.tm_hour, current.tm_min,
			current.tm_mday, current.tm_mon + 1,
			current.tm_year + 1900);
	}





	duration operator - (const datetime& other) const {

		int thisMinutes =
			d.daysBackToDefault() * 1440 +
			t.get_hours() * 60 +
			t.get_minutes();

		int otherMinutes =
			other.d.daysBackToDefault() * 1440 +
			other.t.get_hours() * 60 +
			other.t.get_minutes();

		return duration::fromMinutes(thisMinutes - otherMinutes);
	}
	datetime operator + (const duration& d) const {
		int totalMinutes = d.totalMinutes();

		date newDate = this->d;
		Time newTime = t;

		int currentMinutes = newTime.get_hours() * 60 + newTime.get_minutes();
		int result = currentMinutes + totalMinutes;

		int extraDays = result / 1440;
		result %= 1440;
		if (result < 0) {
			result += 1440;
			extraDays--;
		}

		newTime = Time::fromMinutes(result);
		newDate = newDate + duration::fromDays(extraDays);

		return datetime(newTime.get_hours(), newTime.get_minutes(),
			newDate.get_day(), newDate.get_month(), newDate.get_year());
	}


	datetime operator - (const duration& d) const {
		return *this + -d;
	}
};
