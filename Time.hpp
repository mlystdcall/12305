#ifndef TIME_HPP
#define TIME_HPP

#include<iostream>
#include "Date.hpp"

class Time{
	enum{TOTAL_MINUTES = 1440}; //24 * 60
public:
	Date date;
	int hour, minute;
	Time(Date d = Date(), int h = 0, int m = 0) : date(d), hour(h), minute(m) {}
	Time operator + (const int num) const; //num >= 0
	bool operator < (const Time& b) const;
	int operator - (const Time& b) const;
	friend std::ostream &operator<<(std::ostream &os, const Time &time);
	friend std::istream &operator>>(std::istream &os, Time &time);
	bool exist() const;
};

#endif //TIME_HPP