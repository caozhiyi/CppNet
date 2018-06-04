#include <ctime>
#include <chrono>

#include "Time.h"

CTime::CTime() {
	Now();
}

CTime::~CTime() {

}

CTime::CTime(CTime const& t) {
	memcpy(this, &t, sizeof(*this));
}

std::string CTime::GetDateStr() {
	char tmp[32];
	sprintf_s(tmp, "%04d%02d%02d", (_tm.tm_year + 1900), _tm.tm_mon + 1, _tm.tm_mday);
	return tmp;
}

int CTime::GetDate() {
	char tmp[32];
	sprintf_s(tmp, "%04d%02d%02d", (_tm.tm_year + 1900), _tm.tm_mon + 1, _tm.tm_mday);
	return atoi(tmp);
}

int CTime::GetYearDay() {
	return _tm.tm_yday;
}

int CTime::GetMonthDay() {
	return _tm.tm_mday;
}

int CTime::GetWeekDay() {
	return _tm.tm_wday;
}

int CTime::GetMonth() {
	return _tm.tm_mon;
}

int CTime::GetYear() {
	return _tm.tm_year + 1900;
}

int CTime::GetHour() {
	return _tm.tm_hour;
}

int CTime::GetMin() {
	return _tm.tm_min;
}

int CTime::GetSec() {
	return _tm.tm_sec;
}

int CTime::GetMsec() {
	return _time;
}

std::string CTime::GetFormatTime() {
	char res[32] = { 0 };
	sprintf_s(res, "%04d %02d %02d-%02d:%02d:%02d", (_tm.tm_year + 1900), _tm.tm_mon + 1, _tm.tm_mday, _tm.tm_hour, _tm.tm_min, _tm.tm_sec);
	return std::move(std::string(res));
}

bool CTime::GetFormatTime(char* res, int len) {
	if (len < 32) {
		return false;
	}
	snprintf(res, 32, "%04d %02d %02d-%02d:%02d:%02d", (_tm.tm_year + 1900), _tm.tm_mon + 1, _tm.tm_mday, _tm.tm_hour, _tm.tm_min, _tm.tm_sec);
	return true;
}

void CTime::Now() {
	_time = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
	time_t it = _time / 1000;
	{
		std::unique_lock<std::mutex> lock(_mutex);
		_tm = *std::localtime(&it);
	}
}

bool CTime::operator==(CTime const& t) {
	return _time == t._time;
}

bool CTime::operator>(CTime const& t) {
	return _time > t._time;
}

bool CTime::operator>=(CTime const& t) {
	return _time >= t._time;
}

bool CTime::operator<(CTime const& t) {
	return _time < t._time;
}

bool CTime::operator<=(CTime const& t) {
	return _time <= t._time;
}