#include "Log.h"
#include "Timer.h"
#include "EventHandler.h"

using namespace cppnet;

CTimer::CTimer() : _pool(new base::CMemoryPool(1024, 20)) {

}

CTimer::~CTimer() {

}

uint64_t CTimer::AddTimer(uint32_t interval, const std::function<void(void*)>& call_back, void* param, bool always) {
    base::CMemSharePtr<CTimerEvent> timer_event = base::MakeNewSharedPtr<CTimerEvent>(_pool.get());
    timer_event->_timer_call_back = call_back;
    timer_event->_interval        = interval;
    timer_event->_event_flag      |= EVENT_TIMER;
    timer_event->_timer_param     = param;

    if (always) {
        timer_event->_event_flag |= EVENT_TIMER_ALWAYS;
    }

    _AddTimer(interval, timer_event, timer_event->_timer_id);

    _fix_timer_id_map[timer_event->_timer_id] = timer_event;

    return timer_event->_timer_id;
}

uint64_t CTimer::AddTimer(uint32_t interval, base::CMemSharePtr<CTimerEvent>& event) {
    event->_interval = interval;
    event->_event_flag |= EVENT_TIMER;

    _AddTimer(interval, event, event->_timer_id);

    _fix_timer_id_map[event->_timer_id] = event;

    return event->_timer_id;
}

uint64_t CTimer::AddTimer(uint32_t interval, base::CMemSharePtr<CEventHandler>& event) {
    base::CMemSharePtr<CTimerEvent> timer_event = base::MakeNewSharedPtr<CTimerEvent>(_pool.get());
    timer_event->_interval   = interval;
    timer_event->_event_flag |= EVENT_TIMER | event->_event_flag_set;
    timer_event->_event      = event;

    _AddTimer(interval, timer_event, timer_event->_timer_id);

    _fix_timer_id_map[timer_event->_timer_id] = timer_event;

    return timer_event->_timer_id;
}

bool CTimer::DelTimer(uint64_t timerid) {
    std::unique_lock<std::recursive_mutex> lock(_mutex);
    auto iter = _fix_timer_id_map.find(timerid);
    if (iter == _fix_timer_id_map.end()) {
        return false;
    }

    auto timer_event_ptr = iter->second.Lock();
    if (timer_event_ptr) {
        auto timer_iter = _timer_map.find(timer_event_ptr->_timer_id);
        if (timer_iter != _timer_map.end()) {
            _timer_map.erase(timer_iter);
            return true;
        }
    }

    return false;
}

uint32_t CTimer::TimeoutCheck(std::vector<base::CMemSharePtr<CTimerEvent>>& res) {
    _time.Now();
    return TimeoutCheck(_time.GetMsec(), res);
}

uint32_t CTimer::TimeoutCheck(uint64_t nowtime, std::vector<base::CMemSharePtr<CTimerEvent>>& res) {
    uint64_t recent_timeout = 0;
    std::vector<base::CMemSharePtr<CTimerEvent>> always_timer;
    std::unique_lock<std::recursive_mutex> lock(_mutex);

    for (auto iter = _timer_map.begin(); iter != _timer_map.end();) {
        if (iter->first <= nowtime) {
            if (iter->second->_event_flag & EVENT_TIMER_ALWAYS) {
                always_timer.push_back(iter->second);
            }
            res.push_back(iter->second);
            iter = _timer_map.erase(iter);

        } else {
            recent_timeout = iter->first - nowtime;
            break;
        }
    }

    // add to timer again
    if (!always_timer.empty()) {
        for (auto iter = always_timer.begin(); iter != always_timer.end(); ++iter) {
            AddTimer((*iter)->_interval, (*iter));
        }
    }
    return recent_timeout;
}

uint32_t CTimer::GetTimerNum() {
    std::unique_lock<std::recursive_mutex> lock(_mutex);
    return _timer_map.size();
}

void CTimer::_AddTimer(uint32_t interval, const base::CMemSharePtr<CTimerEvent>& t, uint64_t& id) {
    _time.Now();
    uint64_t nowtime = _time.GetMsec();
    uint64_t key = nowtime + interval;

    std::unique_lock<std::recursive_mutex> lock(_mutex);
    while (_timer_map.count(key)) {
        key++;
    }
    _timer_map[key] = t;
    id = key;
}

void CTimer::_AddTimer(uint32_t interval, base::CMemSharePtr<CTimerEvent>& event) {
    event->_interval = interval;
    event->_event_flag |= EVENT_TIMER;
    _AddTimer(interval, event, event->_timer_id);
}