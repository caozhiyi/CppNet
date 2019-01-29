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
class CTimerEvent;
class CTimer
{
public:
	CTimer();
	~CTimer();

	//add a timer. return the timer id
    unsigned int AddTimer(unsigned int interval, const std::function<void(void*)>& call_back, void* param, bool always = false);
    unsigned int AddTimer(unsigned int interval, CMemSharePtr<CTimerEvent>& event);
    unsigned int AddTimer(unsigned int interval, CMemSharePtr<CEventHandler>& event);

	//delete a timer
	bool DelTimer(unsigned int timerid);

	//check timer whether or not to go out of time. if timeout.
	//res return all timeout timer.
	//return the recent timeout time. if there is no one, return 0
	unsigned int TimeoutCheck(std::vector<CMemSharePtr<CTimerEvent>>& res);
	unsigned int TimeoutCheck(unsigned int nowtime, std::vector<CMemSharePtr<CTimerEvent>>& res);

	int GetTimerNum();
private:
    void _AddTimer(unsigned int interval, const CMemSharePtr<CTimerEvent>& t, unsigned int& id);
    void _AddTimer(unsigned int interval, CMemSharePtr<CTimerEvent>& event);
private:
    std::shared_ptr<CMemoryPool>	    _pool;
    std::recursive_mutex			    _mutex;
	CTimeTool							_time;
	std::map<unsigned int, CMemSharePtr<CTimerEvent>>	_timer_map;
    std::map<unsigned int, CMemWeakPtr<CTimerEvent>>	_fix_timer_id_map;
};

#endif