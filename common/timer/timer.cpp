
// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#include "timer.h"
#include "timer_1ms.h"
#include "timer_container.h"

namespace cppnet {

std::unique_ptr<Timer> MakeTimer50Ms() {
    return std::unique_ptr<Timer>(new Timer1ms());
}

std::unique_ptr<Timer> MakeTimer1Sec() {
    auto sub = MakeTimer50Ms();
    return std::unique_ptr<Timer>(new TimerContainer(std::move(sub), TC_50MS, TC_1SEC));
}

std::unique_ptr<Timer> MakeTimer1Min() {
    auto sub = MakeTimer1Sec();
    return std::unique_ptr<Timer>(new TimerContainer(std::move(sub), TC_1SEC, TC_1MIN));
}

std::unique_ptr<Timer> MakeTimer1Hour() {
    auto sub = MakeTimer1Min();
    return std::unique_ptr<Timer>(new TimerContainer(std::move(sub), TC_1MIN, TC_1HOUR));
}

}