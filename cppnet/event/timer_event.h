// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef CPPNET_EVENT_TIMER_EVENT
#define CPPNET_EVENT_TIMER_EVENT

#include <memory>
#include <atomic>
#include "event_interface.h"
#include "include/cppnet_type.h"
#include "common/timer/timer_solt.h"

namespace cppnet {

class TimerEvent: 
    public Event,
    public TimerSolt {

public:
    TimerEvent(): _timer_id(0) {}
    ~TimerEvent() {}
    
    void SetTimerCallBack(const user_timer_call_back& cb, void* param);

    void OnTimer();
private:
    uint64_t _timer_id;
    user_timer_call_back _timer_cb;

};

}

#endif