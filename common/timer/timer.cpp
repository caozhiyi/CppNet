
#include "timer.h"
#include "timer_1ms.h"
#include "timer_container.h"

namespace cppnet {

std::shared_ptr<Timer> MakeTimer50Ms() {
    return std::make_shared<Timer1ms>();
}

std::shared_ptr<Timer> MakeTimer1Sec() {
    auto sub = MakeTimer50Ms();
    return std::make_shared<TimerContainer>(sub, TC_50MS, TC_1SEC);
}

std::shared_ptr<Timer> MakeTimer1Min() {
    auto sub = MakeTimer1Sec();
    return std::make_shared<TimerContainer>(sub, TC_1SEC, TC_1MIN);
}

std::shared_ptr<Timer> MakeTimer1Hour() {
    auto sub = MakeTimer1Min();
    return std::make_shared<TimerContainer>(sub, TC_1MIN, TC_1HOUR);
}

}