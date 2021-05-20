// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef CPPNET_SOCKET_SOCKET_INTERFACE
#define CPPNET_SOCKET_SOCKET_INTERFACE

#include <memory>
#include <cstdint>
#include <unordered_map>

#include "common/network/address.h"
#ifdef __win__
#include "common/structure/thread_safe_unordered_map.h"
#endif

namespace cppnet {

class Buffer;
class Address;
class CppNetBase;
class Dispatcher;
class AlloterWrap;
class EventActions;
class Socket { 
public:
    Socket(): _sock(0), _addr() {}
    Socket(std::shared_ptr<AlloterWrap> alloter):
        _sock(0), _alloter(alloter) {}
    Socket(uint64_t sock, std::shared_ptr<AlloterWrap> alloter): 
        _sock(sock), _alloter(alloter) {}
    virtual ~Socket() {}

    void SetSocket(const uint64_t& sock) { _sock = sock; }
    uint64_t GetSocket() { return _sock; }

    void SetAddress(const Address& addr) { _addr = addr; }
    const Address& GetAddress() const { return _addr; }

    void SetCppNetBase(std::shared_ptr<CppNetBase> base) { _cppnet_base = base; }
    const std::shared_ptr<CppNetBase> GetCppNetBase() const { return _cppnet_base.lock(); }

    void SetEventActions(std::weak_ptr<EventActions> actions) { _event_actions = actions; }
    const std::shared_ptr<EventActions> GetEventActions() const { return _event_actions.lock(); }

    void SetDispatcher(std::shared_ptr<Dispatcher> dis) { _dispatcher = dis; }
    std::shared_ptr<Dispatcher> GetDispatcher() { return _dispatcher.lock(); }

    void SetAlloter(std::shared_ptr<AlloterWrap> alloter) { _alloter = alloter; }
    std::shared_ptr<AlloterWrap> GetAlloter() { return _alloter; }

protected:
    uint64_t _sock;
    Address _addr;

    std::shared_ptr<AlloterWrap> _alloter;
    
    std::weak_ptr<CppNetBase>   _cppnet_base;
    std::weak_ptr<EventActions> _event_actions;
    std::weak_ptr<Dispatcher>   _dispatcher;

#ifdef __win__
    static ThreadSafeUnorderedMap<uint64_t, std::shared_ptr<Socket>> __all_socket_map;
#else
    static thread_local std::unordered_map<uint64_t, std::shared_ptr<Socket>> __all_socket_map;
#endif
};

}

#endif