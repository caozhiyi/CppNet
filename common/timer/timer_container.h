// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef COMMON_TIMER_TIMER_CONTAINER
#define COMMON_TIMER_TIMER_CONTAINER

#include <map>
#include <list>
#include <memory>
#include <unordered_map>

#include "../util/bitmap.h"
#include "timer_interface.h"

namespace cppnet {

// timer wheel container, include a sub timer.
// if set timeout time is little than accuracy, will be added to sub timer wheel.
// It inherits from the timer interface, 
// so can set another timer wheel container to sub timer to support more timer set.
// More timer define see timer.h file.
class TimerContainer: 
    public Timer {

public:
    TimerContainer(std::shared_ptr<TimerContainer> sub_timer, TIME_UNIT unit, TIME_UNIT max);
    ~TimerContainer();

    bool AddTimer(std::weak_ptr<TimerSolt> t, uint32_t time, bool always = false);
    bool RmTimer(std::weak_ptr<TimerSolt> t);

    // get min next time out time
    // return
    // >= 0 : the next time
    //  < 0 : has no timer
    int32_t MinTime();
    // return the timer wheel current time
    int32_t CurrentTimer();
    // timer wheel run time 
    // return carry
    uint32_t TimerRun(uint32_t step);

    bool Empty();
    void Clear();

    // get current timer wheel timeout time
    int32_t LocalMinTime();
    bool InnerAddTimer(std::shared_ptr<TimerSolt> ptr, uint32_t time);

    void SetRootTimer(std::shared_ptr<TimerContainer> timer) { _root_timer = timer; }

protected:
    uint16_t TimeUnit2TimeType(TIME_UNIT tu);
    uint32_t GetIndexLeftInterval(uint16_t index);
    void GetIndexTimer(std::vector<std::weak_ptr<TimerSolt>>& run_timer_solts, 
        std::vector<std::weak_ptr<TimerSolt>>& sub_timer_solts, uint32_t index, uint32_t time_pass);
    void DoTimer(std::vector<std::weak_ptr<TimerSolt>>& run_timer_solts,
        std::vector<std::weak_ptr<TimerSolt>>& sub_timer_solts);

protected:
    TIME_UNIT _time_unit;
    uint32_t  _size;
    uint32_t  _timer_max;

    uint32_t _cur_time;
    Bitmap   _bitmap;
    std::weak_ptr<TimerContainer>   _root_timer;
    std::shared_ptr<TimerContainer> _sub_timer;
    std::unordered_map<uint32_t, std::map<uint32_t, std::list<std::weak_ptr<TimerSolt>>>> _timer_wheel;
    //std::unordered_map<uint32_t, std::list<std::weak_ptr<TimerSolt>>> _timer_wheel;
};

}

#endif