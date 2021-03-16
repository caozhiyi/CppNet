#ifndef NET_EVENT_TIMER_EVENT
#define NET_EVENT_TIMER_EVENT

#include <memory>

#include "event_interface.h"
#include "common/timer/timer_solt.h"

namespace cppnet {
   
    template<typename T>
    class TimerEvent: public TimerSolt, public Event {
    public:
        TimerEvent(std::function<void(T)> cb, T param):
            _call_back(cb),
            _param(param) {}
        ~TimerEvent() {}

        void OnTimer() {
            _call_back(_param);
        }

    private:
        T _param;
        std::function<void(T)>  _call_back;   // only timer event
    };
}

#endif