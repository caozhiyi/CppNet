// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#include "timer_solt.h"

namespace cppnet {

TimerSolt::TimerSolt():
    _total_interval(0),
    _left_interval(0) {

}

void TimerSolt::SetInterval(uint32_t interval) {
    _total_interval = interval;
    _left_interval = interval;
}

uint32_t TimerSolt::GetTotalInterval() { 
    _total_interval &= ~(TSF_ALWAYS | TSF_IN_TIMER);
    return _total_interval; 
}

uint32_t TimerSolt::GetLeftInterval() { 
    return _left_interval; 
}

void TimerSolt::ResetTime() {
    _left_interval = _total_interval;
    _cur_index = 0;
}

uint32_t TimerSolt::TimePass(uint32_t time) {
    _left_interval -= time;
    return _left_interval;
}

void TimerSolt::SetInTimer() {
    _total_interval |= TSF_IN_TIMER;
}

bool TimerSolt::IsInTimer() {
    return _total_interval & TSF_IN_TIMER;
}

void TimerSolt::RmInTimer() {
    _total_interval &= ~TSF_IN_TIMER;
}

void TimerSolt::SetAlways() {
    _total_interval |= TSF_ALWAYS;
}

bool TimerSolt::IsAlways() {
    return _total_interval & TSF_ALWAYS;
}

void TimerSolt::RmAlways() {
    _total_interval &= ~TSF_ALWAYS;
}

void TimerSolt::SetCurIndex(uint16_t index, uint16_t type) {
    _cur_index = index | type;
}

void TimerSolt::GetCurIndex(uint16_t& index, uint16_t& type) {
    index = _cur_index & ~TIT_MUSK;
    type = _cur_index & TIT_MUSK;
}

}