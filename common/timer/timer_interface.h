// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef COMMON_TIMER_TIMER_INTERFACE
#define COMMON_TIMER_TIMER_INTERFACE

#include <memory>
#include <cstdint>

namespace cppnet {

// time unit
enum TIME_UNIT: uint32_t {
    TU_MILLISECOND = 1,
    TU_SECOND      = TU_MILLISECOND * 1000,
    TU_MINUTE      = TU_SECOND * 60,
    TU_HOUR        = TU_MINUTE * 60,
};

enum TIMER_CODE {
    NO_TIMER = -1 // don't have timer
};

class TimerSlot;

// timer interface, timer inherits from this.
class Timer {
public:
    Timer() {}
    ~Timer() {}

    virtual bool AddTimer(std::weak_ptr<TimerSlot> t, uint32_t time, bool always = false) = 0;
    virtual bool RmTimer(std::weak_ptr<TimerSlot> t) = 0;

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

    virtual bool Empty() = 0;
};

}

#endif