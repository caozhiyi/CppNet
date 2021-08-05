// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)
// Copyright <caozhiyi5@gmail.com>


#include "cppnet/socket/rw_socket.h"
#include "cppnet/event/timer_event.h"

#include "foundation/log/log.h"

namespace cppnet {

void TimerEvent::SetTimerCallBack(const user_timer_call_back& cb, void* param) {
  timer_cb_ = cb;
  SetData(param);
}

void TimerEvent::OnTimer() {
  if (GetType() & ET_USER_TIMER) {
    timer_cb_(GetData());

  } else if (GetType() & ET_TIMER) {
    auto sock = GetSocket();
    auto rw_sock = std::dynamic_pointer_cast<RWSocket>(sock);
    rw_sock->OnTimer();

  } else {
    fdan::LOG_ERROR("invalid timer type. type:%d", GetType());
  }
}

}  // namespace cppnet
