#ifndef INCLUDE_CPPNET_DEFINE
#define INCLUDE_CPPNET_DEFINE

#include <memory>
#include <cstdint>
#include <functional>

#include "common/buffer/buffer_interface.h"

namespace cppnet {
    // socket
    class CNSocket;
    typedef std::shared_ptr<CNSocket> Handle;
    // call back define
    // param : param is setted when set
    typedef std::function<void(void* param)>                                       timer_call_back;

    // handle : handle of socket
    // err    : error code
    typedef std::function<void(Handle handle, uint32_t err)>                       connect_call_back;

    // handle : handle of socket
    // len    : sended date len
    // err    : error code
    typedef std::function<void(Handle handle, uint32_t len, uint32_t err)>         write_call_back;

    // handle : handle of socket
    // data   : point to recv data buffer
    // len    : recv data len
    // err    : error code
    // continue_read : continue post read event
    typedef std::function<void(Handle handle, std::shared_ptr<Buffer> data, 
                        uint32_t len, uint32_t err)>                               read_call_back;
    
    // error code
    enum CPPNET_ERROR_CODE {
        CEC_SUCCESS                = 0,    // success.
        CEC_TIMEOUT                = 1,    // the event time out call back.
        CEC_CLOSED                 = 2,    // remote close the socket.
        CEC_INVALID_HANDLE         = 3,    // invalid cppnet handle, can find in socket manager.
        CEC_FAILED                 = 4,    // call function failed.
        CEC_CONNECT_BREAK          = 5,    // connect break.
        CEC_CONNECT_REFUSE         = 6,    // remote refuse connect or server not exist.
    };
}

#endif