#include <assert.h>
#include <algorithm> // for min

#include "timer_container.h"

namespace cppnet {

TimerContainer::TimerContainer(std::shared_ptr<Timer> t, TIMER_CAPACITY accuracy, TIMER_CAPACITY capacity) :
    _sub_timer(t),
    _cur_index(0),
    _accuracy(accuracy),
    _capacity(capacity) {
    assert(t);
    _max_size = capacity/accuracy;
    _timer_wheel.resize(_max_size);
    _bitmap.Init(_max_size);
}
TimerContainer::~TimerContainer() {

}

bool TimerContainer::AddTimer(std::weak_ptr<TimerSolt> t, uint32_t time, bool always) {
    if (time >= _capacity) {
        return false;
    }

    if (time < _accuracy) {
        return _sub_timer->AddTimer(t, time, always);
    }

    auto ptr = t.lock();
    if (!ptr) {
        return false;
    }

    // only can set once
    if (ptr->IsInTimer()) {
        return true;
    }
    ptr->SetTimer();

    // set always flag
    if (always) {
        ptr->SetAlways(_accuracy);
    }

    ptr->SetInterval(time);

    // relative to current time
    time += _cur_index * _accuracy;
    if (time > _capacity) {
        time %= _capacity;
    }

    // insert into timer
    uint32_t index = ptr->SetIndex(time);
    _timer_wheel[index].push_back(t);
    return _bitmap.Insert(index);
}

bool TimerContainer::RmTimer(std::weak_ptr<TimerSolt> t) {
    auto ptr = t.lock();
    if (!ptr) {
        return false;
    }

    bool ret = _bitmap.Remove(ptr->GetIndex(_accuracy)) && _sub_timer->RmTimer(t);
    // clear timer solt flag
    ptr->RmTimer();
    return ret;
}

int32_t TimerContainer::MinTime() {
    int32_t sub_time = _sub_timer->MinTime();
    int32_t local_time = LocalMinTime();

    // return less time
    if (sub_time > 0 && local_time > 0) {
        return std::min(sub_time, local_time);

    } else if (sub_time > 0) {
        return sub_time;

    } else {
        return local_time;
    }
}

int32_t TimerContainer::CurrentTimer() {
    return _cur_index * _accuracy + _sub_timer->CurrentTimer();
}

uint32_t TimerContainer::TimerRun(uint32_t time) {
    time = time % _capacity;
    _cur_index += time / _accuracy;

    uint32_t carry = _sub_timer->TimerRun(time % _accuracy);
    _cur_index += carry;

    uint32_t ret = 0;
    if (_cur_index >= _max_size) {
        _cur_index %= _max_size;
        ret++;
    }

    auto bucket = _timer_wheel[_cur_index];
    if (bucket.size() > 0) {
        for (auto iter = bucket.begin(); iter != bucket.end();) {
            auto ptr = (iter)->lock();
            if (ptr && ptr->IsInTimer()) {
                ptr->OnTimer();
            }
            // remove from current bucket
            iter = bucket.erase(iter);
            _bitmap.Remove(ptr->GetIndex(_accuracy));
            if (!ptr->IsAlways(_accuracy) || !ptr->IsInTimer()) {
                ptr->RmTimer();
                continue;

            } else {
                // add to timer again
                uint16_t next_index = (ptr->GetInterval() / _accuracy) + _cur_index;
                if (next_index >= _max_size) {
                    next_index = next_index % _max_size;
                }
                AddTimerByIndex(ptr, next_index);
            }
        }
    }
    return ret;
}

void TimerContainer::AddTimerByIndex(std::weak_ptr<TimerSolt> t, uint8_t index) {
    auto ptr = t.lock();
    if (!ptr) {
        return;
    }

    ptr->SetIndex(index, _accuracy);
    _timer_wheel[index].push_back(t);
    _bitmap.Insert(index);
}

int32_t TimerContainer::LocalMinTime() {
    int32_t sub_runned_time = _sub_timer->CurrentTimer();
    int32_t next_setp = _bitmap.GetMinAfter(_cur_index);
    if (next_setp >= 0) {
        return (next_setp - _cur_index) * _accuracy - sub_runned_time;
    }

    if (_cur_index > 0) {
        next_setp = _bitmap.GetMinAfter(0);
    }
    
    if (next_setp >= 0) {
        return (next_setp + _max_size - _cur_index) * _accuracy - sub_runned_time;
    }

    return NO_TIMER;
}

}