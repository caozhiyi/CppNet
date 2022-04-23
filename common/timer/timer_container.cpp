// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#include <algorithm>
#include "timer_slot.h"
#include "timer_container.h"

namespace cppnet {

TimerContainer::TimerContainer(std::shared_ptr<TimerContainer> sub_timer, TIME_UNIT unit, TIME_UNIT max):
    _time_unit(unit),
    _size(max / unit),
    _timer_max(max),
    _cur_time(0),
    _sub_timer(sub_timer) {

    _bitmap.Init(_size);
}

TimerContainer::~TimerContainer() {

}

bool TimerContainer::AddTimer(std::weak_ptr<TimerSlot> t, uint32_t time, bool always) {
    if (time >= _timer_max) {
        return false;
    }

    auto ptr = t.lock();
    if (!ptr) {
        return false;
    }

    if (time < _time_unit && _sub_timer) {
        return _sub_timer->AddTimer(t, time, always);
    }

    // set current timer unit interval
    if (!ptr->IsInTimer()) {
        ptr->SetInterval(time);
        ptr->SetInTimer();
        if (always) {
            ptr->SetAlways();
        }

    } else {
        return false;
    }

    return InnerAddTimer(ptr, time);
}

bool TimerContainer::RmTimer(std::weak_ptr<TimerSlot> t) {
    auto ptr = t.lock();
    if (!ptr) {
        return false;
    }

    uint16_t cur_index = 0;
    uint16_t type = 0;
    ptr->GetCurIndex(cur_index, type);
    if (type == TimeUnit2TimeType(_time_unit)) {
        ptr->RmInTimer();
        _bitmap.Remove(cur_index);
        uint32_t left_time = ptr->GetLeftInterval();
        auto timer_map = _timer_wheel.find(cur_index);
        if (timer_map == _timer_wheel.end()) {
            return true;
        }
        // don't have sub timer
        if (!_sub_timer) {
            left_time = 0;
        }
        auto timer_list = timer_map->second.find(left_time);
        if (timer_list == timer_map->second.end()) {
            return false;
        }
        for (auto timer = timer_list->second.begin(); timer != timer_list->second.end(); ++timer) {
            auto target = timer->lock();
            if (target == ptr) {
                timer_list->second.erase(timer);
                return true;
            }
        }
        return true;
    }

    if (_sub_timer) {
        return _sub_timer->RmTimer(t);
    }
    return false;
}

int32_t TimerContainer::MinTime() {
    uint32_t past_time = 0;
    int32_t sub_time = 0;
    int32_t local_time = LocalMinTime();

    if (_sub_timer) {
        sub_time = _sub_timer->MinTime();
        past_time += _sub_timer->CurrentTimer();

        if (sub_time > 0) {
            local_time = local_time - past_time;
            if (local_time > 0) {
                return std::min(local_time, sub_time);
            }
            return sub_time;

        } else if (local_time > 0){
            return local_time - past_time;
        }
    }

    return local_time;
}

int32_t TimerContainer::CurrentTimer() {
    uint32_t ret = _cur_time * _time_unit;
    if (_sub_timer) {
        ret += _sub_timer->CurrentTimer();
    }
    return ret;
}

uint32_t TimerContainer::TimerRun(uint32_t time) {
    uint32_t time_pass = time / _time_unit;
    uint32_t left_time = time % _time_unit;
    bool do_timer = time_pass > 0;

    if (left_time > 0) {
        uint32_t sub_run_step = _sub_timer->TimerRun(left_time);
        if (sub_run_step > 0) {
            do_timer = true;
        }
        _cur_time += sub_run_step;
    }

    if (!do_timer) {
        uint32_t run_setp = 0;
        if (_cur_time >= _size) {
            _cur_time -= _size;
            run_setp++;
        }
        return run_setp;
    }

    std::vector<std::weak_ptr<TimerSlot>> run_timer_solts;
    std::vector<std::weak_ptr<TimerSlot>> sub_timer_solts;

    uint32_t prev_time = _cur_time;
    _cur_time += time_pass;
    while(1) {
        int32_t next_time = _bitmap.GetMinAfter(prev_time);
        if (next_time < 0) {
            break;
        }
        if (next_time > (int32_t)_cur_time) {
            break;
        }
        GetIndexTimer(run_timer_solts, sub_timer_solts, next_time, left_time);
        prev_time = next_time + 1;
    }

    uint32_t run_setp = 0;
    if (_cur_time >= _size) {
        _cur_time -= _size;
        run_setp++;
    }

    if (run_setp > 0) {
        prev_time = 0;
        while (1) {
            int32_t next_time = _bitmap.GetMinAfter(prev_time);
            if (next_time < 0) {
                break;
            }
            if (next_time > (int32_t)_cur_time) {
                break;
            }
            GetIndexTimer(run_timer_solts, sub_timer_solts, next_time, left_time);
            prev_time = next_time + 1;
        }
    }

    // timer call back
    DoTimer(run_timer_solts, sub_timer_solts);

    return run_setp;
}

bool TimerContainer::Empty() {
    return _bitmap.Empty();
}

void TimerContainer::Clear() {
    _bitmap.Clear();
    _timer_wheel.clear();
    if (_sub_timer) {
        _sub_timer->Clear();
    }
}

int32_t TimerContainer::LocalMinTime() {
    int32_t next_time = _bitmap.GetMinAfter(_cur_time);
    if (next_time >= 0) {
        return (next_time - _cur_time) * _time_unit + GetIndexLeftInterval(next_time);
    }

    if (_cur_time > 0) {
        next_time = _bitmap.GetMinAfter(0);
        
        if (next_time >= 0) {
            return (next_time + _size - _cur_time) * _time_unit + GetIndexLeftInterval(next_time);
        }
    }
    return NO_TIMER;
}

bool TimerContainer::InnerAddTimer(std::shared_ptr<TimerSlot> ptr, uint32_t time) {
    if (time > _timer_max) {
        return false;
    }
    
    uint16_t cur_index = time / _time_unit + _cur_time;
    if (cur_index >= _size) {
        cur_index -= _size;
    }
    uint32_t left_time = time % _time_unit;
    // don't have sub timer
    if (!_sub_timer) {
        left_time = 0;
    }
    ptr->SetCurIndex(cur_index, TimeUnit2TimeType(_time_unit));
    ptr->TimePass(time - left_time);

    _timer_wheel[cur_index][left_time].push_back(ptr);
    return _bitmap.Insert(cur_index);
}

uint16_t TimerContainer::TimeUnit2TimeType(TIME_UNIT tu) {
    switch (tu)
    {
    case TU_MILLISECOND:
        return TIT_MILLISECOND;
    case TU_SECOND:
        return TIT_SECOND;
    case TU_MINUTE:
        return TIT_MINUTE;
    default:
        throw "invalid time unit";
    }
}

uint32_t TimerContainer::GetIndexLeftInterval(uint16_t index) {
    uint32_t left_interval = 0;
    auto timer_map = _timer_wheel.find(index);
    if (timer_map != _timer_wheel.end() && !timer_map->second.empty()) {
        left_interval = timer_map->second.begin()->first;
    }
    return left_interval;
}

void TimerContainer::GetIndexTimer(std::vector<std::weak_ptr<TimerSlot>>& run_timer_solts, 
    std::vector<std::weak_ptr<TimerSlot>>& sub_timer_solts, uint32_t index, uint32_t time_pass) {
    auto bucket_iter = _timer_wheel.find(index);
    if (bucket_iter == _timer_wheel.end()) {
        return;
    }
    auto& bucket = bucket_iter->second;

    for (auto timer_list = bucket.begin(); timer_list != bucket.end(); timer_list++) {
        if (timer_list->first == 0) {
            run_timer_solts.insert(run_timer_solts.end(), timer_list->second.begin(), timer_list->second.end());
            continue;
        }

        if (timer_list->first <= time_pass) {
            run_timer_solts.insert(run_timer_solts.end(), timer_list->second.begin(), timer_list->second.end());
            continue;
        }

        for (auto timer = timer_list->second.begin(); timer != timer_list->second.end(); timer++) {
            auto target = timer->lock();
            sub_timer_solts.push_back(target);
        }
    }

    _bitmap.Remove(_cur_time);
    _timer_wheel.erase(bucket_iter);
}

void TimerContainer::DoTimer(std::vector<std::weak_ptr<TimerSlot>>& run_timer_solts,
    std::vector<std::weak_ptr<TimerSlot>>& sub_timer_solts) {
    // timer call back
    for (auto iter = run_timer_solts.begin(); iter != run_timer_solts.end(); iter++) {
        auto ptr = iter->lock();
        if (!ptr) {
            continue;
        }
        ptr->OnTimer();
        ptr->RmInTimer();

        // add timer again
        if (ptr->IsAlways()) {
            ptr->ResetTime();
            auto root_timer = _root_timer.lock();
            if (root_timer) {
                root_timer->AddTimer(ptr, ptr->GetTotalInterval(), ptr->IsAlways());
            }
            else {
                AddTimer(ptr, ptr->GetTotalInterval(), ptr->IsAlways());
            }
        }
    }

    // add sub timer
    if (!_sub_timer) {
        return;
    }
    for (auto iter = sub_timer_solts.begin(); iter != sub_timer_solts.end(); iter++) {
        auto ptr = iter->lock();
        if (!ptr) {
            continue;
        }
        _sub_timer->InnerAddTimer(ptr, ptr->GetLeftInterval());
    }
}

}