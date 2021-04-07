// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef CPPNET_EVENT_WIN_ACCEPT_EVENT
#define CPPNET_EVENT_WIN_ACCEPT_EVENT

#include "cppnet/cppnet_config.h"
#include "cppnet/event/event_interface.h"

namespace cppnet {

class AcceptEvent:
    public Event{
public:
    AcceptEvent():
        _client_sock(0),
        _buf_offset(0),
        _index_in_socket(0) {
        memset(_buf, 0, __iocp_buff_size);
    }

    virtual ~AcceptEvent() {}

    char* GetBuf() { return _buf; }
    
    void SetBufOffset(uint32_t offset) { _buf_offset = offset; }
    uint32_t GetBufOffset() { return _buf_offset; }

    void SetClientSocket(uint64_t sock) { _client_sock = sock; }
    uint64_t GetClientSocket() { return _client_sock;  }

	void SetIndex(uint16_t index) { _index_in_socket = index; }
    uint16_t GetIndex() { return _index_in_socket; }

private:
	uint64_t _client_sock;
	char     _buf[__iocp_buff_size];
	uint32_t _buf_offset;
    uint16_t _index_in_socket;
};

}

#endif