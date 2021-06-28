
// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#include "timer.h"
#include "timer_container.h"

namespace cppnet {

std::shared_ptr<Timer> MakeTimer1Sec() {
    return std::make_shared<TimerContainer>(nullptr, TU_MILLISECOND, TU_SECOND);
}

std::shared_ptr<Timer> MakeTimer1Min() {
    auto sec_sub = std::make_shared<TimerContainer>(nullptr, TU_MILLISECOND, TU_SECOND);
    auto timer = std::make_shared<TimerContainer>(sec_sub, TU_SECOND, TU_MINUTE);
    sec_sub->SetRootTimer(timer);
    return timer;
}

std::shared_ptr<Timer> MakeTimer1Hour() {
    auto sec_sub = std::make_shared<TimerContainer>(nullptr, TU_MILLISECOND, TU_SECOND);
    auto min_sub = std::make_shared<TimerContainer>(sec_sub, TU_SECOND, TU_MINUTE);
    auto timer = std::make_shared<TimerContainer>(min_sub, TU_MINUTE, TU_HOUR);
    sec_sub->SetRootTimer(timer);
    min_sub->SetRootTimer(timer);

    return timer;
}

}