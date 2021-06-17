// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef COMMON_TIMER_TIMER_CONTAINER
#define COMMON_TIMER_TIMER_CONTAINER

#include "timer_1ms.h"

namespace cppnet {

// timer wheel container, include a sub timer.
// if set timeout time is little than accuracy, will be added to sub timer wheel.
// It inherits from the timer interface, 
// so can set another timer wheel container to sub timer to support more timer set.
// More timer define see timer.h file.
class TimerContainer: 
    public Timer {

public:
    TimerContainer(std::unique_ptr<Timer> t, TIMER_CAPACITY accuracy, TIMER_CAPACITY capacity);
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
    
private:
    // add timer by index. only set current time wheel
    void AddTimerByIndex(std::weak_ptr<TimerSolt> t, uint8_t index);
    // get current timer wheel timeout time
    int32_t LocalMinTime();
    
private:
    std::vector<std::list<std::weak_ptr<TimerSolt>>> _timer_wheel;
    std::unique_ptr<Timer> _sub_timer;
    uint32_t _cur_index;
    Bitmap _bitmap;

    TIMER_CAPACITY _accuracy;
    TIMER_CAPACITY _capacity;
    uint32_t _max_size;
};

}

#endif