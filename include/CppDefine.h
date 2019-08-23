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
                        uint32_t len, uint32_t err, bool& continue_read)>                                  read_call_back;
    
    // error code
    enum CPPNET_ERROR_CODE {
        CEC_SUCCESS                = 0x0100,
        CEC_TIMEOUT                = 0x0200,
        CEC_CLOSED                 = 0x0400,
        CEC_DONE                   = 0x0800,
        CEC_INVALID_HANDLE         = 0x0001,
        CEC_FAILED                 = 0x0002,
        CEC_CONNECT_BREAK          = 0x0003,
    };
}

#endif