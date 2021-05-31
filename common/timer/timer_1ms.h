// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef COMMON_TIMER_TIMER_1MS
#define COMMON_TIMER_TIMER_1MS

#include <list>
#include <vector>

#include "timer_solt.h"
#include "timer_interface.h"
#include "common/util/bitmap.h"

namespace cppnet {

// the base timer wheel, 
// time unit is millisecond, 
// and max support 50 milliseconds.
class Timer1ms: 
    public Timer {

public:
    Timer1ms();
    ~Timer1ms();

    bool AddTimer(std::weak_ptr<TimerSolt> t, uint32_t time, bool always = false);
    bool RmTimer(std::weak_ptr<TimerSolt> t);

    // get min next time out time
    // return: 
    // >= 0  : the next time
    //  < 0  : has no timer
    int32_t MinTime();

    // return the timer wheel current time
    int32_t CurrentTimer();

    // timer wheel run time 
    // return carry
    uint32_t TimerRun(uint32_t time);
    // add timer by index. only set current time wheel
    void AddTimerByIndex(std::weak_ptr<TimerSolt> t, uint8_t index);
    
private:
    std::vector<std::list<std::weak_ptr<TimerSolt>>> _timer_wheel;
    uint32_t _cur_index;
    Bitmap _bitmap;
};

}

#endif