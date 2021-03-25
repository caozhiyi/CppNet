#include "accept_event.h"

namespace cppnet {

AcceptEvent::AcceptEvent(): 
    _client_sock(0),
    _buf_offset(0) {
    memset(_buf, 0, __iocp_buff_size);
}

}