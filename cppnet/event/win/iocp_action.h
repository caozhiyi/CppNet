// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef CPPNET_EVENT_WIN_IOCP_ACTION
#define CPPNET_EVENT_WIN_IOCP_ACTION

#include "../action_interface.h"

namespace cppnet {

struct EventOverlapped;

// epoll event interface
class IOCPEventActions:
    public EventActions {
public:
    IOCPEventActions();
    virtual ~IOCPEventActions();

    virtual bool Init(uint32_t thread_num = 0);
    virtual bool Dealloc();
    // net io event
    virtual bool AddSendEvent(std::shared_ptr<Event>& event);
    virtual bool AddRecvEvent(std::shared_ptr<Event>& event);
    virtual bool AddAcceptEvent(std::shared_ptr<Event>& event);

    virtual bool AddConnection(std::shared_ptr<Event>& event, Address& address);
    virtual bool AddDisconnection(std::shared_ptr<Event>& event);

    virtual bool DelEvent(std::shared_ptr<Event>& event);
    // io thread process
    virtual void ProcessEvent(int32_t wait_ms);
    // weak up net io thread
    virtual void Wakeup();
private:
    bool AddToIOCP(uint64_t sock);
    void DoEvent(EventOverlapped *socket_context, uint32_t bytes);

protected:
    void*     _iocp_handler;
};

}

#endif