#ifndef COMMON_TIMER_TIMER
#define COMMON_TIMER_TIMER

#include <memory>
#include "timer_interface.h"

namespace cppnet {

std::shared_ptr<Timer> MakeTimer50Ms();

std::shared_ptr<Timer> MakeTimer1Sec();

std::shared_ptr<Timer> MakeTimer1Min();

std::shared_ptr<Timer> MakeTimer1Hour();

}

#endif