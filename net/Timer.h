#ifndef HEADER_CTIMER
#define HEADER_CTIMER

#include <map>
#include <vector>

#include "TimeTool.h"
#include "Single.h"
#include "EventHandler.h"
#include "Buffer.h"
#include "PoolSharedPtr.h"

class CEventHandler;
struct TimerEvent {
	int							_event_flag;
	CMemSharePtr<CEventHandler> _event;
	TimerEvent() : _event_flag(0) {}
	TimerEvent(CMemSharePtr<CEventHandler>& event) : _event_flag(0), _event(event){}
};

class CTimer
{
public:
	CTimer();
	~CTimer();

	//add a timer. return the timer id
	void AddTimer(unsigned int interval, TimerEvent& t);
	void AddTimer(unsigned int interval, unsigned int nowtime, TimerEvent& t);
	void AddTimer(unsigned int interval, int event_flag, CMemSharePtr<CEventHandler>& event);
	void AddTimer(unsigned int interval, int event_flag, unsigned int nowtime, CMemSharePtr<CEventHandler>& event);

	//delete a timer
	bool DelTimer(unsigned int timerid);
	bool DelTimer(CMemSharePtr<CEventHandler>& event);

	//check timer whether or not to go out of time. if timeout.
	//res return all timeout timer.
	//return the recent timeout time. if there is no one, return 0
	unsigned int TimeoutCheck(std::vector<TimerEvent>& res);
	unsigned int TimeoutCheck(unsigned int nowtime, std::vector<TimerEvent>& res);

	int GetTimerNum();

private:
	std::mutex							_mutex;
	CTimeTool								_time;
	std::map<unsigned int, TimerEvent>	_timer_map;
};

#endif