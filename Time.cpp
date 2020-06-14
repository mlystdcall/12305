#include "includes.hpp"
//#include "Time.hpp"
//#include<cstdio>

Time Time::operator + (const int num) const{
	if(num == 0) return *this;
	int NUM = num;
	Date d = date + (NUM / TOTAL_MINUTES);
	NUM %= TOTAL_MINUTES;
	int h = hour, m = minute;
	if(h * 60 + m + NUM >= TOTAL_MINUTES) d = d + 1;
	h = (h + NUM / 60) % 24;
	NUM = NUM % 60;
	if(m + NUM >= 60) h = (h + 1) % 24;
	m = (m + NUM) % 60;
	return Time(d, h, m);
}

bool Time::operator < (const Time& b) const{
	if(this->date < b.date) return true;
	if(b.date < this->date) return false;
	if(this->hour < b.hour) return true;
	if(this->hour > b.hour) return false;
	if(this->minute < b.minute) return true;
	return false;
}

int Time::operator - (const Time& b) const{
	Time A = *this, B = b;
	int rs = (A.date - B.date) * TOTAL_MINUTES;
	rs += A.hour * 60 + A.minute;
	rs -= B.hour * 60 + B.minute;
	return rs;
}

std::ostream &operator<<(std::ostream &os, const Time &time) {
	if(time.hour < 10) os << "0";
	os << time.hour << ":";
	if(time.minute < 10) os << "0";
	os << time.minute;
	return os;
}

std::istream &operator>>(std::istream &os, Time &time) {
	char c;
	os >> time.hour >> c >> time.minute;
	return os;
}

bool Time::exist() const{
	if(!date.exist()) return false;
	if(hour < 0 || hour >= 24) return false;
	if(minute < 0 || minute >= 60) return false;
	return true;
}