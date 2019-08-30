#include <ctime>
#include <chrono>
#include <cstring>

#include "TimeTool.h"

base::CTimeTool::CTimeTool() {
	Now();
}

base::CTimeTool::~CTimeTool() {

}

base::CTimeTool::CTimeTool(CTimeTool const& t) {
	memcpy(this, &t, sizeof(*this));
}

std::string base::CTimeTool::GetDateStr() {
	char tmp[32];
	sprintf(tmp, "%04d%02d%02d", (_tm.tm_year + 1900), _tm.tm_mon + 1, _tm.tm_mday);
	return tmp;
}

int base::CTimeTool::GetDate() {
	char tmp[32];
	sprintf(tmp, "%04d%02d%02d", (_tm.tm_year + 1900), _tm.tm_mon + 1, _tm.tm_mday);
	return atoi(tmp);
}

int base::CTimeTool::GetYearDay() {
	return _tm.tm_yday;
}

int base::CTimeTool::GetMonthDay() {
	return _tm.tm_mday;
}

int base::CTimeTool::GetWeekDay() {
	return _tm.tm_wday;
}

int base::CTimeTool::GetMonth() {
	return _tm.tm_mon;
}

int base::CTimeTool::GetYear() {
	return _tm.tm_year + 1900;
}

int base::CTimeTool::GetHour() {
	return _tm.tm_hour;
}

int base::CTimeTool::GetMin() {
	return _tm.tm_min;
}

int base::CTimeTool::GetSec() {
	return _tm.tm_sec;
}

time_t base::CTimeTool::GetMsec() {
	return _time;
}

std::string base::CTimeTool::GetFormatTime() {
	char res[32] = { 0 };
	sprintf(res, "%04d %02d %02d-%02d:%02d:%02d", (_tm.tm_year + 1900), _tm.tm_mon + 1, _tm.tm_mday, _tm.tm_hour, _tm.tm_min, _tm.tm_sec);
	return std::move(std::string(res));
}

bool base::CTimeTool::GetFormatTime(char* res, int len) {
	if (len < 32) {
		return false;
	}
	snprintf(res, 32, "%04d %02d %02d-%02d:%02d:%02d", (_tm.tm_year + 1900), _tm.tm_mon + 1, _tm.tm_mday, _tm.tm_hour, _tm.tm_min, _tm.tm_sec);
	return true;
}

void base::CTimeTool::Now() {
	_time = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
	time_t it = _time / 1000;

	std::unique_lock<std::mutex> lock(_mutex);
	_tm = *std::gmtime(&it);
}

bool base::CTimeTool::operator==(CTimeTool const& t) {
	return _time == t._time;
}

bool base::CTimeTool::operator>(CTimeTool const& t) {
	return _time > t._time;
}

bool base::CTimeTool::operator>=(CTimeTool const& t) {
	return _time >= t._time;
}

bool base::CTimeTool::operator<(CTimeTool const& t) {
	return _time < t._time;
}

bool base::CTimeTool::operator<=(CTimeTool const& t) {
	return _time <= t._time;
}