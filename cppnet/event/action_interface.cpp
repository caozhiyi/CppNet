#include "action_interface.h"
#include "common/timer/timer.h"
#include "common/timer/timer_interface.h"

namespace cppnet {

EventActions::EventActions(): 
    _run(false),
    _cur_utc_time(0) {
    _timer = MakeTimer1Min();
}

EventActions::~EventActions() {

}

bool EventActions::AddTimerEvent(std::shared_ptr<TimeSolt> t, uint32_t interval, bool always) {
    return _timer->AddTimer(t, interval, always);
}

bool EventActions::EventActions::RemoveTimerEvent(std::shared_ptr<TimeSolt> t) {
    return _timer->RmTimer(t);
}

}
