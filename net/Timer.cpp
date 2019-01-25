#include "Timer.h"
#include "EventHandler.h"
#include "Log.h"

CTimer::CTimer() {

}

CTimer::~CTimer() {

}

void CTimer::AddTimer(unsigned int interval, const TimerEvent& t, unsigned int& id) {
    _time.Now();
    unsigned int nowtime = _time.GetMsec();
    unsigned int key = nowtime + interval;

    std::unique_lock<std::mutex> lock(_mutex);
    if (_timer_map.count(key)) {
        key++;
    }
    _timer_map[key] = t;
    id = key;
}

void CTimer::AddTimer(unsigned int interval, TimerEvent& t) {
	_time.Now();
	unsigned int nowtime = _time.GetMsec();
	unsigned int key = nowtime + interval;
	t._event->_timer_id = key;

	std::unique_lock<std::mutex> lock(_mutex);
	if (_timer_map.count(key)) {
		key++;
	}
	_timer_map[key] = t;
}

void CTimer::AddTimer(unsigned int interval, unsigned int nowtime, TimerEvent& t) {
	unsigned int key = nowtime + interval;
	t._event->_timer_id = key;

	std::unique_lock<std::mutex> lock(_mutex);
	if (_timer_map.count(key)) {
		key++;
	}
	_timer_map[key] = t;
}

void CTimer::AddTimer(unsigned int interval, int event_flag, CMemSharePtr<CEventHandler>& event) {
	_time.Now();
	unsigned int nowtime = _time.GetMsec();
	unsigned int key = nowtime + interval;

	std::unique_lock<std::mutex> lock(_mutex);
	if (_timer_map.count(key)) {
		key++;
	}
	_timer_map[key] = TimerEvent(event);
	_timer_map[key]._event_flag = event_flag;
	_timer_map[key]._event->_timer_id = key;
}

void CTimer::AddTimer(unsigned int interval, int event_flag, unsigned int nowtime, CMemSharePtr<CEventHandler>& event) {
	unsigned int key = nowtime + interval;

	std::unique_lock<std::mutex> lock(_mutex);
	if (_timer_map.count(key)) {
		key++;
	}
	_timer_map[key] = TimerEvent(event);
	_timer_map[key]._event_flag = event_flag;
	_timer_map[key]._event->_timer_id = key;
}

bool CTimer::DelTimer(unsigned int timerid) {
	std::unique_lock<std::mutex> lock(_mutex);
	auto iter = _timer_map.find(timerid);
	if (iter != _timer_map.end()) {
		iter->second._event->_timer_set = false;
		_timer_map.erase(iter);
		return true;
	}
	return false;
}

bool CTimer::DelTimer(CMemSharePtr<CEventHandler>& event) {
	std::unique_lock<std::mutex> lock(_mutex);
	auto iter = _timer_map.find(event->_timer_id);
	if (iter != _timer_map.end()) {
		iter->second._event->_timer_set = false;
		_timer_map.erase(iter);
		return true;
	}
	return false;
}

unsigned int CTimer::TimeoutCheck(std::vector<TimerEvent>& res) {
	_time.Now();
	unsigned int nowtime = _time.GetMsec();
	unsigned int recent_timeout = 0;
	std::unique_lock<std::mutex> lock(_mutex);
	for (auto iter = _timer_map.begin(); iter != _timer_map.end();) {
		if (iter->first <= nowtime) {
            if (!(iter->second._event_flag & EVENT_TIMER)) {
                iter->second._event->_timer_out = true;
            }
			res.push_back(iter->second);
			iter = _timer_map.erase(iter);
           
        } else {
			recent_timeout = iter->first - nowtime;
			break;
		}
	}
	return recent_timeout;
}

unsigned int CTimer::TimeoutCheck(unsigned int nowtime, std::vector<TimerEvent>& res) {
	unsigned int recent_timeout = 0;
	std::unique_lock<std::mutex> lock(_mutex);
    for (auto iter = _timer_map.begin(); iter != _timer_map.end();) {
        if (iter->first <= nowtime) {
            if (!(iter->second._event_flag & EVENT_TIMER)) {
                iter->second._event->_timer_out = true;
            }
            res.push_back(iter->second);
            iter = _timer_map.erase(iter);

        } else {
            recent_timeout = iter->first - nowtime;
            break;
        }
    }
	return recent_timeout;
}

int CTimer::GetTimerNum() {
	std::unique_lock<std::mutex> lock(_mutex);
	return _timer_map.size();
}