// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#include "timer_event.h"
#include "common/log/log.h"
#include "cppnet/socket/rw_socket.h"

namespace cppnet {

void TimerEvent::SetTimerCallBack(const user_timer_call_back& cb, void* param) {
    _timer_cb = cb;
    SetData(param);
}

void TimerEvent::OnTimer() {
    if (GetType() & ET_USER_TIMER) {
        _timer_cb(GetData());

    } else if (GetType() & ET_TIMER) {
        auto sock = GetSocket();
        auto rw_sock = dynamic_cast<RWSocket*>(sock);
        rw_sock->OnTimer();

    } else {
        LOG_ERROR("invalid timer type. type:%d", GetType());
    }
}

}