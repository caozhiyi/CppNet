#ifndef HEADER_NET_CTIMER
#define HEADER_NET_CTIMER

#include <map>
#include <vector>

#include "TimeTool.h"
#include "Single.h"
#include "EventHandler.h"
#include "Buffer.h"
#include "PoolSharedPtr.h"

namespace cppnet {

    class CEventHandler;
    struct CTimerEvent;
    class CTimer
    {
    public:
        CTimer();
        ~CTimer();

        //add a timer. return the timer id
        uint64_t AddTimer(uint32_t interval, const std::function<void(void*)>& call_back, void* param, bool always = false);
        uint64_t AddTimer(uint32_t interval, base::CMemSharePtr<CTimerEvent>& event);
        uint64_t AddTimer(uint32_t interval, base::CMemSharePtr<CEventHandler>& event);

        //delete a timer
        bool DelTimer(uint64_t timerid);

        //check timer whether or not to go out of time. if timeout.
        //res return all timeout timer.
        //return the recent timeout time. if there is no one, return 0
        uint32_t TimeoutCheck(std::vector<base::CMemSharePtr<CTimerEvent>>& res);
        uint32_t TimeoutCheck(uint64_t nowtime, std::vector<base::CMemSharePtr<CTimerEvent>>& res);

        // return number of event in timer
        uint32_t GetTimerNum();
    private:
        // add timer event to event actions
        void _AddTimer(uint32_t interval, const base::CMemSharePtr<CTimerEvent>& t, uint64_t& id);
        void _AddTimer(uint32_t interval, base::CMemSharePtr<CTimerEvent>& event);
    private:
        std::shared_ptr<base::CMemoryPool>	    _pool;
        std::recursive_mutex			        _mutex;
        base::CTimeTool							_time;
        std::map<uint64_t, base::CMemSharePtr<CTimerEvent>>	_timer_map;
        std::map<uint64_t, base::CMemWeakPtr<CTimerEvent>>	_fix_timer_id_map;
    };

}
#endif