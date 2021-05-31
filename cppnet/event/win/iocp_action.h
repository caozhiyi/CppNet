// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef CPPNET_EVENT_WIN_IOCP_ACTION
#define CPPNET_EVENT_WIN_IOCP_ACTION

#include <memory>
#include <WS2tcpip.h>
#include "cppnet/event/action_interface.h"
#include "common/structure/thread_safe_unordered_map.h"

namespace cppnet {

enum IOCP_NOTIFY_CODE {
    INC_WEAK_UP           = 0xAAAAFFFF,
    INC_CONNECTION_BREAK  = 0x100,
    INC_CONNECTION_REFUSE = 0x200,
    INC_CONNECTION_CLOSE  = 0x400,
};

struct EventOverlapped {
    OVERLAPPED    _overlapped;
    uint32_t      _event_type;
    void*         _event;

    EventOverlapped(): 
        _event_type(0),
        _event(nullptr){
        memset(&_overlapped, 0, sizeof(_overlapped));
    }

    ~EventOverlapped() {}
};

class Socket;
// IOCP event interface
class IOCPEventActions:
    public EventActions {

public:
    IOCPEventActions();
    virtual ~IOCPEventActions();

    virtual bool Init(uint32_t thread_num = 0);
    virtual bool Dealloc();
    // net IO event
    virtual bool AddSendEvent(Event* event);
    virtual bool AddRecvEvent(Event* event);
    virtual bool AddAcceptEvent(Event* event);
    virtual bool AddConnection(Event* event, Address& address);
    virtual bool AddDisconnection(Event* event);

    virtual bool DelEvent(Event* event);
    // IO thread process
    virtual void ProcessEvent(int32_t wait_ms);
    // weak up net IO thread
    virtual void Wakeup();

    bool AddToIOCP(uint64_t sock);

private:
    void DoEvent(EventOverlapped *socket_context, uint32_t bytes);

protected:
    void*     _iocp_handler;
    static ThreadSafeUnorderedMap<uint64_t, std::shared_ptr<Socket>> __connecting_socket_map;
};

}

#endif