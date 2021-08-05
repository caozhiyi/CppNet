// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)
// Copyright <caozhiyi5@gmail.com>

#ifndef CPPNET_EVENT_TIMER_EVENT_H_
#define CPPNET_EVENT_TIMER_EVENT_H_

#include <memory>
#include <atomic>
#include "include/cppnet_type.h"
#include "foundation/timer/timer_solt.h"
#include "cppnet/event/event_interface.h"

namespace cppnet {

class TimerEvent:
  public Event,
  public fdan::TimerSolt {
 public:
  TimerEvent(): timer_id_(0) {}
  ~TimerEvent() {}

  void SetTimerCallBack(const user_timer_call_back& cb, void* param);

  void OnTimer();
 private:
  uint64_t timer_id_;
  user_timer_call_back timer_cb_;
};

}  // namespace cppnet

#endif  // CPPNET_EVENT_TIMER_EVENT_H_
