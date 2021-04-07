// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef INCLUDE_CPPNET
#define INCLUDE_CPPNET

#include <string>
#include <memory>
#include "cppnet_type.h"

namespace cppnet {

class CppNetBase;
// cppnet instace
class CppNet {
public:
    CppNet();
    ~CppNet();
    // common
    // init cppnet library.
    // thread_num : the number of running threads.
    void Init(int32_t thread_num);
    void Destory();

    // thread join
    void Join();

    // must set callback before listen
    void SetReadCallback(const read_call_back& cb);
    void SetWriteCallback(const write_call_back& cb);
    void SetDisconnectionCallback(const connect_call_back& cb);

    // return timer id
    uint64_t AddTimer(int32_t interval, const user_timer_call_back& cb, void* param = nullptr, bool always = false);
    void RemoveTimer(uint64_t timer_id);

    //server
    void SetAcceptCallback(const connect_call_back& cb);
    bool ListenAndAccept(const std::string& ip, int16_t port);

    //client
    void SetConnectionCallback(const connect_call_back& cb);
    bool Connection(const std::string& ip, int16_t port);

private:
    std::shared_ptr<CppNetBase> _cppnet_base;
};

}

#endif