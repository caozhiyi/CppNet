// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef NET_EVENT_LINUX_EPOLL_ACTION
#define NET_EVENT_LINUX_EPOLL_ACTION

#include <mutex>
#include <sys/epoll.h>
#include "../action_interface.h"

namespace cppnet {

// epoll event interface
class EpollEventActions:
    public EventActions {

public:
    EpollEventActions();
    virtual ~EpollEventActions();

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
    void OnEvent(std::vector<epoll_event>& event_vec, int16_t num);
    bool AddEvent(epoll_event* ev, int32_t event_flag, uint64_t sock, bool in_actions);

protected:
    std::mutex  _mutex;
    int32_t     _epoll_handler;
    uint32_t    _pipe[2];
    epoll_event _pipe_content;
    std::vector<epoll_event> _active_list;

};

}

#endif