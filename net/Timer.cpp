#include "Timer.h"
#include "EventHandler.h"
#include "Log.h"

CTimer::CTimer() : _pool(new CMemoryPool(1024, 20)) {

}

CTimer::~CTimer() {

}

unsigned int CTimer::AddTimer(unsigned int interval, const std::function<void(void*)>& call_back, void* param, bool always) {
    CMemSharePtr<CTimerEvent> timer_event = MakeNewSharedPtr<CTimerEvent>(_pool.get());
    timer_event->_timer_call_back = call_back;
    timer_event->_interval    = interval;
    timer_event->_event_flag  |= EVENT_TIMER;
    timer_event->_timer_param = param;
    if (always) {
        timer_event->_event_flag |= EVENT_TIMER_ALWAYS;
    }
    _AddTimer(interval, timer_event, timer_event->_timer_id);
    _fix_timer_id_map[timer_event->_timer_id] = timer_event;
    return timer_event->_timer_id;
}

unsigned int CTimer::AddTimer(unsigned int interval, CMemSharePtr<CTimerEvent>& event) {
    event->_interval = interval;
    event->_event_flag |= EVENT_TIMER;
    _AddTimer(interval, event, event->_timer_id);
    _fix_timer_id_map[event->_timer_id] = event;
    return event->_timer_id;
}

unsigned int CTimer::AddTimer(unsigned int interval, CMemSharePtr<CEventHandler>& event) {
    CMemSharePtr<CTimerEvent> timer_event = MakeNewSharedPtr<CTimerEvent>(_pool.get());
    timer_event->_interval = interval;
    timer_event->_event_flag |= EVENT_TIMER | event->_event_flag_set;
    timer_event->_event = event;
    _AddTimer(interval, timer_event, timer_event->_timer_id);
    _fix_timer_id_map[timer_event->_timer_id] = timer_event;
    return timer_event->_timer_id;
}

bool CTimer::DelTimer(unsigned int timerid) {
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

unsigned int CTimer::TimeoutCheck(std::vector<CMemSharePtr<CTimerEvent>>& res) {
	_time.Now();
	unsigned int nowtime = _time.GetMsec();
	unsigned int recent_timeout = 0;
    std::vector<CMemSharePtr<CTimerEvent>> always_timer;
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
    if (!always_timer.empty()) {
        for (auto iter = always_timer.begin(); iter != always_timer.end(); ++iter) {
            AddTimer((*iter)->_interval, (*iter));
        }
    }
	return recent_timeout;
}

unsigned int CTimer::TimeoutCheck(unsigned int nowtime, std::vector<CMemSharePtr<CTimerEvent>>& res) {
	unsigned int recent_timeout = 0;
    std::vector<CMemSharePtr<CTimerEvent>> always_timer;
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
    if (!always_timer.empty()) {
        for (auto iter = always_timer.begin(); iter != always_timer.end(); ++iter) {
            AddTimer((*iter)->_interval, (*iter));
        }
    }
	return recent_timeout;
}

int CTimer::GetTimerNum() {
    std::unique_lock<std::recursive_mutex> lock(_mutex);
	return _timer_map.size();
}

void CTimer::_AddTimer(unsigned int interval, const CMemSharePtr<CTimerEvent>& t, unsigned int& id) {
    _time.Now();
    unsigned int nowtime = _time.GetMsec();
    unsigned int key = nowtime + interval;

    std::unique_lock<std::recursive_mutex> lock(_mutex);
    if (_timer_map.count(key)) {
        key++;
    }
    _timer_map[key] = t;
    id = key;
}

void CTimer::_AddTimer(unsigned int interval, CMemSharePtr<CTimerEvent>& event) {
    event->_interval = interval;
    event->_event_flag |= EVENT_TIMER;
    _AddTimer(interval, event, event->_timer_id);
}