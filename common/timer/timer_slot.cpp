// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#include "common/timer/timer_slot.h"

namespace cppnet {

TimerSlot::TimerSlot():
    _total_interval(0),
    _left_interval(0) {

}

void TimerSlot::SetInterval(uint32_t interval) {
    _total_interval = interval;
    _left_interval = interval;
}

uint32_t TimerSlot::GetTotalInterval() { 
    _total_interval &= ~(TSF_ALWAYS | TSF_IN_TIMER);
    return _total_interval; 
}

uint32_t TimerSlot::GetLeftInterval() { 
    return _left_interval; 
}

void TimerSlot::ResetTime() {
    _left_interval = _total_interval;
    _cur_index = 0;
}

uint32_t TimerSlot::TimePass(uint32_t time) {
    _left_interval -= time;
    return _left_interval;
}

void TimerSlot::SetInTimer() {
    _total_interval |= TSF_IN_TIMER;
}

bool TimerSlot::IsInTimer() {
    return _total_interval & TSF_IN_TIMER;
}

void TimerSlot::RmInTimer() {
    _total_interval &= ~TSF_IN_TIMER;
}

void TimerSlot::SetAlways() {
    _total_interval |= TSF_ALWAYS;
}

bool TimerSlot::IsAlways() {
    return _total_interval & TSF_ALWAYS;
}

void TimerSlot::RmAlways() {
    _total_interval &= ~TSF_ALWAYS;
}

void TimerSlot::SetCurIndex(uint16_t index, uint16_t type) {
    _cur_index = index | type;
}

void TimerSlot::GetCurIndex(uint16_t& index, uint16_t& type) {
    index = _cur_index & ~TIT_MUSK;
    type = _cur_index & TIT_MUSK;
}

}