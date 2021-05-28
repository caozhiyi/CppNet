// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef CPPNET_CPPNET_BASE
#define CPPNET_CPPNET_BASE

#include <vector>
#include <memory>
#include <string>
#include "include/cppnet_type.h"

namespace cppnet {

class InnerBuffer;
class RWSocket;
class Dispatcher;
class RangeRandom;

class CppNetBase: public std::enable_shared_from_this<CppNetBase> {
public:
    CppNetBase();
    ~CppNetBase();
    // common
    void Init(uint32_t thread_num);
    void Dealloc();
    void Join();

    // set call back
    void SetReadCallback(const read_call_back& cb) { _read_cb = cb; }
    void SetWriteCallback(const write_call_back& cb) { _write_cb = cb; }
    void SetDisconnectionCallback(const connect_call_back& cb) { _disconnect_cb = cb; }
    void SetTimerCallback(const timer_call_back& cb) { _timer_cb = cb; }

    // about timer
    uint64_t AddTimer(uint32_t interval, const user_timer_call_back& cb, void* param = nullptr, bool always = false);
    void RemoveTimer(uint64_t timer_id);

    //server
    void SetAcceptCallback(const connect_call_back& cb) { _accept_cb = cb; }
    bool ListenAndAccept(const std::string& ip, uint16_t port);

    //client
    void SetConnectionCallback(const connect_call_back& cb) { _connect_cb = cb; }
    bool Connection(const std::string& ip, uint16_t port);

    // call back
    void OnTimer(std::shared_ptr<RWSocket> sock);
    void OnAccept(std::shared_ptr<RWSocket> sock);
    void OnRead(std::shared_ptr<RWSocket> sock, std::shared_ptr<InnerBuffer> buffer, uint32_t len);
    void OnWrite(std::shared_ptr<RWSocket> sock, uint32_t len);
    void OnConnect(std::shared_ptr<RWSocket> sock, uint16_t err);
    void OnDisConnect(std::shared_ptr<RWSocket> sock, uint16_t err);

private:
    timer_call_back    _timer_cb;
    read_call_back     _read_cb;
    write_call_back    _write_cb;
    connect_call_back  _connect_cb;
    connect_call_back  _disconnect_cb;
    connect_call_back  _accept_cb;

    std::shared_ptr<RangeRandom> _random;
    std::vector<std::shared_ptr<Dispatcher>> _dispatchers;
};

}
#endif