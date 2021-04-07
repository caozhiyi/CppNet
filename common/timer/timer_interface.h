// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef COMMON_TIMER_TIMER_INTERFACE
#define COMMON_TIMER_TIMER_INTERFACE

#include <memory>
#include <cstdint>

namespace cppnet {

// time unit
enum TIME_UNIT {
    MILLISECOND = 1,
    SECOND      = 1000,
    MINUTE      = 60 * 1000,
};

enum TIMER_CODE {
    NO_TIMER = -1 // don't have timer
};

enum TIMER_CAPACITY {
    TC_1MS   = 1 * MILLISECOND,
    TC_50MS  = 50 * MILLISECOND,
    TC_1SEC  = 1 * SECOND,
    TC_1MIN  = 1 * MINUTE,
    TC_1HOUR = 60 * MINUTE
};

class TimerSolt;

// timer interface, timer inherits from this.
class Timer {
public:
    Timer() {}
    ~Timer() {}

    virtual bool AddTimer(std::weak_ptr<TimerSolt> t, uint32_t time, bool always = false) = 0;
    virtual bool RmTimer(std::weak_ptr<TimerSolt> t) = 0;

    // get min next time out time
    // return: 
    // >= 0  : the next time
    //  < 0  : has no timer
    virtual int32_t MinTime() = 0;

    // return the timer wheel current time
    virtual int32_t CurrentTimer() = 0;

    // timer wheel run time 
    // return carry
    virtual uint32_t TimerRun(uint32_t time) = 0;

    // only internal used by timer
    // add timer by index. only set current time wheel
    virtual void AddTimerByIndex(std::weak_ptr<TimerSolt> t, uint8_t index) = 0;
};

}

#endif