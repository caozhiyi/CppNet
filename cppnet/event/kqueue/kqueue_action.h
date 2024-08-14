// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef NET_EVENT_KQUEUE_KQUEUE_ACTION
#define NET_EVENT_KQUEUE_KQUEUE_ACTION

#include <mutex>
#include <vector>
#include <cstdint>
#include <sys/event.h> // for kqueue
#include "cppnet/event/action_interface.h"

namespace cppnet {

// kqueue event interface
class KqueueEventActions: public EventActions {
public:
    KqueueEventActions();
    virtual ~KqueueEventActions();

    virtual bool Init(uint32_t thread_num = 0);
    virtual bool Dealloc();
    // net io event
    virtual bool AddSendEvent(Event* event);
    virtual bool AddRecvEvent(Event* event);
    virtual bool AddAcceptEvent(Event* event);
    virtual bool AddConnection(Event* event, Address& address);
    virtual bool AddDisconnection(Event* event);

    virtual bool DelEvent(Event* event);
    // io thread process
    virtual void ProcessEvent(int32_t wait_ms);
    // weak up net io thread
    virtual void Wakeup();

private:
    void OnEvent(std::vector<struct kevent>& event_vec, int16_t num);

protected:
    std::mutex _mutex;
    int32_t    _kqueue_handler;
    uint32_t   _pipe[2];
    timespec   _kqueue_timeout;
    std::vector<struct kevent> _change_list;
    std::vector<struct kevent> _active_list;
};

}

#endif