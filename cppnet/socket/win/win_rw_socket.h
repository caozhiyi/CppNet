// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef CPPNET_SOCKET_WIN_READ_WRITE_SOCKET
#define CPPNET_SOCKET_WIN_READ_WRITE_SOCKET

#include <mutex>
#include <atomic>
#include <unordered_set>
#include "../rw_socket.h"

namespace cppnet {

class Event;
class WinRWSocket:
    public RWSocket { 

public:
    WinRWSocket(std::shared_ptr<AlloterWrap> alloter);
    WinRWSocket(uint64_t sock, std::shared_ptr<AlloterWrap> alloter);
    virtual ~WinRWSocket();

    virtual void Read();
    virtual bool Write(const char* src, uint32_t len);
    virtual void Connect(const std::string& ip, uint16_t port);
    virtual void Disconnect();

    virtual void OnRead(Event* event, uint32_t len = 0);
    virtual void OnWrite(Event* event, uint32_t len = 0);
    virtual void OnConnect(Event* event, uint16_t err);
    virtual void OnDisConnect(Event* event, uint16_t err);

    void SetShutdown() { _shutdown = true; }
    bool IsShutdown() { return _shutdown; }

private:
    void AddEvent(Event* event);
    void RemvoeEvent(Event* event);
    bool EventEmpty();

private:
    std::atomic_bool _shutdown;
    std::atomic_bool _is_reading;

    std::mutex _event_mutex;
    // all event
    std::unordered_set<Event*> _event_set;
};

}

#endif