// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef INCLUDE_CPPNET
#define INCLUDE_CPPNET

#include <memory>
#include <string>

#include "cppnet_buffer.h"
#include "cppnet_socket.h"
#include "cppnet_type.h"

namespace cppnet {

class CppNetBase;
// cppnet instance
class CppNet {
public:
    CppNet() = default;
    ~CppNet();
    // common
    // init cppnet library.
    // thread_num : the number of running threads.
    void Init(int32_t thread_num = 0);
    void Destory();

    // thread join
    void Join();

    // must set callback before listen
    void SetReadCallback(read_call_back&& cb);
    void SetWriteCallback(write_call_back&& cb);
    void SetDisconnectionCallback(connect_call_back&& cb);

    // if use socket timer, set it
    void SetTimerCallback(timer_call_back&& cb);

    // return timer id
    uint64_t AddTimer(int32_t interval, user_timer_call_back&& cb, void* param = nullptr, bool always = false);
    void RemoveTimer(uint64_t timer_id);

    //server
    void SetAcceptCallback(connect_call_back&& cb);
    bool ListenAndAccept(const std::string& ip, uint16_t port);

    //client
    void SetConnectionCallback(connect_call_back&& cb);
    bool Connection(const std::string& ip, uint16_t port);

private:
    std::shared_ptr<CppNetBase> _cppnet_base;
};

} // namespace cppnet

#endif