// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef INCLUDE_CPPNET_TYPE
#define INCLUDE_CPPNET_TYPE

#include <memory>
#include <cstdint>
#include <functional>

namespace cppnet {

// socket
class Buffer;
class CNSocket;
typedef std::shared_ptr<CNSocket> Handle;
typedef std::shared_ptr<Buffer>   BufferPtr;

// call back define
// param : param is set when call
typedef std::function<void(Handle)>                                            timer_call_back;
typedef std::function<void(void*)>                                             user_timer_call_back;

// handle : handle of socket
// err    : error code
typedef std::function<void(Handle handle, uint32_t err)>                       connect_call_back;

// handle : handle of socket
// len    : send date len
typedef std::function<void(Handle handle, uint32_t len)>                       write_call_back;

// handle : handle of socket
// data   : point to recv data buffer
// len    : recv data len
typedef std::function<void(Handle handle, BufferPtr data, 
                        uint32_t len)>                                         read_call_back;
    
// error code
enum CPPNET_ERROR_CODE {
    CEC_SUCCESS                = 0,    // success.
    CEC_CLOSED                 = 1,    // remote close the socket.
    CEC_CONNECT_BREAK          = 2,    // connect break.
    CEC_CONNECT_REFUSE         = 3,    // remote refuse connect or server not exist.
};

}

#endif