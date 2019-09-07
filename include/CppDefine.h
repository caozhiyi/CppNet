#ifndef HEADER_INCLUDE_CPPNETDEFINE
#define HEADER_INCLUDE_CPPNETDEFINE

#include <functional>
#include "Type.h"
#include "Buffer.h"

namespace cppnet {
    // socket
    typedef uint64_t    Handle;
    // call back define
    // param : param is setted when set
    typedef std::function<void(void* param)>                                                               timer_call_back;

    // handle : handle of socket
    // err    : error code
    typedef std::function<void(const Handle& handle, uint32_t err)>                                        connection_call_back;

    // handle : handle of socket
    // len    : sended date len
    // err    : error code
    typedef std::function<void(const Handle& handle, uint32_t len, uint32_t err)>                          write_call_back;

    // handle : handle of socket
    // data   : point to recv data buffer
    // len    : recv data len
    // err    : error code
    // continue_read : continue post read event
    typedef std::function<void(const Handle& handle, base::CBuffer* data, 
                        uint32_t len, uint32_t err)>                                                       read_call_back;
    
    // error code
    enum CPPNET_ERROR_CODE {
        CEC_SUCCESS                = 1,    // success.
        CEC_TIMEOUT                = 2,    // the event time out call back.
        CEC_CLOSED                 = 3,    // remote close the socket.
        CEC_INVALID_HANDLE         = 4,    // invalid cppnet handle, can find in socket manager.
        CEC_FAILED                 = 5,    // call function failed.
        CEC_CONNECT_BREAK          = 6,    // connect break.
        CEC_CONNECT_REFUSE         = 7     // remote refuse connect or server not exist.
    };
}

#endif