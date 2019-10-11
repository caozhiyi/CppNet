#include <ctime>
#include <chrono>
#include <cstring>

#include "TimeTool.h"

#define SECS_PER_HOUR        (60 * 60)
#define SECS_PER_DAY         (SECS_PER_HOUR * 24)
#define DIV(a, b)            ((a) / (b) - ((a) % (b) < 0))
#define LEAPS_THRU_END_OF(y) (DIV (y, 4) - DIV (y, 100) + DIV (y, 400))

#define __isleap(year) \
    ((year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0))

base::CTimeTool::CTimeTool() : _update_time(true) {
    Now();
}

base::CTimeTool::~CTimeTool() {

}

base::CTimeTool::CTimeTool(CTimeTool const& t) {
    memcpy(this, &t, sizeof(*this));
}

std::string base::CTimeTool::GetDateStr() {
    char tmp[32] = { 0 };
    std::unique_lock<std::mutex> lock(_mutex);
    UpdateTm();
    sprintf(tmp, "%04d%02d%02d", (_tm.tm_year + 1900), _tm.tm_mon + 1, _tm.tm_mday);
    return tmp;
}

int base::CTimeTool::GetDate() {
    char tmp[32] = { 0 };
    std::unique_lock<std::mutex> lock(_mutex);
    UpdateTm();
    sprintf(tmp, "%04d%02d%02d", (_tm.tm_year + 1900), _tm.tm_mon + 1, _tm.tm_mday);
    return atoi(tmp);
}

int base::CTimeTool::GetYearDay() {
    std::unique_lock<std::mutex> lock(_mutex);
    UpdateTm();
    return _tm.tm_yday;
}

int base::CTimeTool::GetMonthDay() {
    std::unique_lock<std::mutex> lock(_mutex);
    UpdateTm();
    return _tm.tm_mday;
}

int base::CTimeTool::GetWeekDay() {
    std::unique_lock<std::mutex> lock(_mutex);
    UpdateTm();
    return _tm.tm_wday;
}

int base::CTimeTool::GetMonth() {
    std::unique_lock<std::mutex> lock(_mutex);
    UpdateTm();
    return _tm.tm_mon;
}

int base::CTimeTool::GetYear() {
    std::unique_lock<std::mutex> lock(_mutex);
    UpdateTm();
    return _tm.tm_year + 1900;
}

int base::CTimeTool::GetHour() {
    std::unique_lock<std::mutex> lock(_mutex);
    UpdateTm();
    return _tm.tm_hour;
}

int base::CTimeTool::GetMin() {
    std::unique_lock<std::mutex> lock(_mutex);
    UpdateTm();
    return _tm.tm_min;
}

int base::CTimeTool::GetSec() {
    std::unique_lock<std::mutex> lock(_mutex);
    UpdateTm();
    return _tm.tm_sec;
}

time_t base::CTimeTool::GetMsec() {
    std::unique_lock<std::mutex> lock(_mutex);
    return _time;
}

std::string base::CTimeTool::GetFormatTime() {
    char res[32] = { 0 };
    std::unique_lock<std::mutex> lock(_mutex);
    UpdateTm();
    sprintf(res, "%04d %02d %02d-%02d:%02d:%02d", (_tm.tm_year + 1900), _tm.tm_mon + 1, _tm.tm_mday, _tm.tm_hour, _tm.tm_min, _tm.tm_sec);
    return std::move(std::string(res));
}

bool base::CTimeTool::GetFormatTime(char* res, int len) {
    if (len < 32) {
        return false;
    }
    std::unique_lock<std::mutex> lock(_mutex);
    UpdateTm();
    snprintf(res, 32, "%04d %02d %02d-%02d:%02d:%02d", (_tm.tm_year + 1900), _tm.tm_mon + 1, _tm.tm_mday, _tm.tm_hour, _tm.tm_min, _tm.tm_sec);
    return true;
}

void base::CTimeTool::Now() {
    std::unique_lock<std::mutex> lock(_mutex);
    _time = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    _update_time = true;
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

time_t base::CTimeTool::mktime64(unsigned int year, unsigned int mon,
                           unsigned int day, unsigned int hour,
                           unsigned int min, unsigned int sec) {
    if (0 >= (int) (mon -= 2)) {    /* 1..12 -> 11,12,1..10 */
             mon += 12;      /* Puts Feb last since it has leap day */
             year -= 1;
        }

    return (((
             (time_t) (year/4 - year/100 + year/400 + 367*mon/12 + day) +
             year*365 - 719499
           )*24 + hour /* now have hours */
        )*60 + min /* now have minutes */
    )*60 + sec; /* finally seconds */
}

int base::CTimeTool::gmtime64(const time_t *t, std::tm *tp) {
    const unsigned short int __mon_yday[2][13] =
    {
        /* Normal years.  */
        { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
        /* Leap years.  */
        { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
    };
    long int days, rem, y;
    const unsigned short int *ip;

    days = *t / SECS_PER_DAY;
    rem = *t % SECS_PER_DAY;
    while (rem < 0)
    {
        rem += SECS_PER_DAY;
        --days;
    }
    while (rem >= SECS_PER_DAY)
    {
        rem -= SECS_PER_DAY;
        ++days;
    }

    tp->tm_hour = rem / SECS_PER_HOUR;
    rem %= SECS_PER_HOUR;
    tp->tm_min = rem / 60;
    tp->tm_sec = rem % 60;
    /* January 1, 1970 was a Thursday.  */
    tp->tm_wday = (4 + days) % 7;
    if (tp->tm_wday < 0)
    tp->tm_wday += 7;
    y = 1970;

    while (days < 0 || days >= (__isleap (y) ? 366 : 365))
    {
        /* Guess a corrected year, assuming 365 days per year.  */
        long int yg = y + days / 365 - (days % 365 < 0);
        /* Adjust DAYS and Y to match the guessed year.  */
        days -= ((yg - y) * 365
               + LEAPS_THRU_END_OF (yg - 1)
               - LEAPS_THRU_END_OF (y - 1));
        y = yg;
    }

    tp->tm_year = y - 1900;
    if (tp->tm_year != y - 1900)
    {
        /* The year cannot be represented due to overflow.  */
        //__set_errno (EOVERFLOW);
        return 0;
    }

    tp->tm_yday = days;
    ip = __mon_yday[__isleap(y)];
    for (y = 11; days < (long int) ip[y]; --y)
        continue;
    days -= ip[y];
    tp->tm_mon = y;
    tp->tm_mday = days + 1;
    return 1;
}

void base::CTimeTool::UpdateTm() {
    if (_update_time) {
        time_t it = _time / 1000;
        gmtime64(&it, &_tm);
        _update_time = false;
    }
}
