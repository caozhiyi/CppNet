#ifndef CPPNET_EVENT_WIN_ACCEPT_EVENT
#define CPPNET_EVENT_WIN_ACCEPT_EVENT

#include "cppnet/cppnet_config.h"
#include "cppnet/event/event_interface.h"

namespace cppnet {

class AcceptEvent:
    public Event{
public:
    AcceptEvent();
    virtual ~AcceptEvent() {}

    char* GetBuf() { return _buf; }
    
    void SetBufOffset(uint32_t offset) { _buf_offset = offset; }
    uint32_t GetBufOffset() { return _buf_offset; }

    void SetClientSocket(uint64_t sock) { _client_sock = sock; }
    uint64_t GetClientSocket() { return _client_sock;  }

private:
    uint64_t _client_sock;
    char     _buf[__iocp_buff_size];
    uint32_t _buf_offset;
};

}

#endif