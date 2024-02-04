// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef INCLUDE_CPPNET_TYPE
#define INCLUDE_CPPNET_TYPE

#include <cstdint>
#include <functional>
#include <memory>

namespace cppnet {

// socket
class Buffer;
class CNSocket;
using Handle = std::shared_ptr<CNSocket>;
using BufferPtr = std::shared_ptr<Buffer>;

// call back define
// param : param is set when call
using timer_call_back = std::function<void (Handle)>;
using user_timer_call_back = std::function<void (void *)>;

// handle : handle of socket
// err    : error code
using connect_call_back = std::function<void (Handle, uint32_t)>;

// handle : handle of socket
// len    : send date len
using write_call_back = std::function<void (Handle, uint32_t)>;

// handle : handle of socket
// data   : point to recv data buffer
// len    : recv data len
using read_call_back = std::function<void (Handle, BufferPtr, uint32_t)>;
    
// error code
enum CPPNET_ERROR_CODE {
    CEC_SUCCESS                = 0,    // success.
    CEC_CLOSED                 = 1,    // remote close the socket.
    CEC_CONNECT_BREAK          = 2,    // connection break.
    CEC_CONNECT_REFUSE         = 3,    // remote refuse connect or server not exist.
};

} // namespace cppnet

#endif