#ifndef HEADER_CTIME
#define HEADER_CTIME

#include <mutex>

class CMemoryPool;
class CTimeTool
{
public:
	CTimeTool();
	~CTimeTool();
	CTimeTool(CTimeTool const& t);

	void Now();
	std::string GetDateStr();
	int GetDate();
	int GetYearDay();
	int GetMonthDay();
	int GetWeekDay();
	int GetMonth();
	int GetYear();
	int GetHour();
	int GetMin();
	int GetSec();
	int GetMsec();

	//return xxxx xx xx-xx:xx:xx
	std::string GetFormatTime();
	//return xxxx xx xx-xx:xx:xx
	bool GetFormatTime(char* res, int len);

	bool operator==(CTimeTool const& t);
	bool operator>(CTimeTool const& t);
	bool operator>=(CTimeTool const& t);
	bool operator<(CTimeTool const& t);
	bool operator<=(CTimeTool const& t);

private:
	time_t		_time;
	std::tm     _tm;
	std::mutex	_mutex;
};

#endif