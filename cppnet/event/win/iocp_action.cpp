#include <MSWSock.h>
#include <winsock2.h>

#include "iocp_action.h"
#include "common/log/log.h"
#include "cppnet/cppnet_config.h"
#include "cppnet/socket/rw_socket.h"
#include "common/buffer/buffer_queue.h"
#include "cppnet/event/event_interface.h"

#pragma comment(lib,"ws2_32.lib")

namespace cppnet {

enum IOCP_NOTIFY_CODE {
    INC_WEAK_UP = 0xAAAAFFFF,
};

struct EventOverlapped {
    OVERLAPPED  _overlapped;
    uint32_t    _event_type;
    void*       _event;

    EventOverlapped() {
        _event_type = 0;
        memset(&_overlapped, 0, sizeof(_overlapped));
    }

    ~EventOverlapped() {}
};

IOCPEventActions::IOCPEventActions():
    _iocp_handler(nullptr) {

}

IOCPEventActions::~IOCPEventActions() {

}


bool IOCPEventActions::Init(uint32_t thread_num) {
    //tell iocp thread num
    _iocp_handler = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, thread_num);
    if (_iocp_handler == INVALID_HANDLE_VALUE) {
        LOG_ERROR("IOCP create io completion port failed!");
        return false;
    }
    return true;
}

bool IOCPEventActions::Dealloc() {
    Wakeup();
    return true;
}

bool IOCPEventActions::AddSendEvent(std::shared_ptr<Event>& event) {
    if (event->GetType() & ET_WRITE) {
        return false;
    }
    
    auto sock = event->GetSocket();
    if (!sock) {
        LOG_WARN("socket is already distroyed! event %s", "AddSendEvent");
        return false;
    }

    if (!(event->GetType() & ET_INACTIONS)) {
        if (!AddToIOCP(sock->GetSocket())) {
            return false;
        }
    }

    EventOverlapped* content = (EventOverlapped*)event->GetData();
    auto rw_sock = std::dynamic_pointer_cast<RWSocket>(sock);
    if (!content) {
        content = rw_sock->GetAlocter()->PoolNew<EventOverlapped>();
        event->SetData(content);
    }

    content->_event = (void*)&event;
    content->_event_type = ET_WRITE;

    auto buffer = rw_sock->GetWriteBuffer();
    std::vector<Iovec> bufs;
    buffer->GetUseMemoryBlock(bufs, __iocp_buff_size);

    int32_t ret = WSASend(sock->GetSocket(), (LPWSABUF)&(*bufs.begin()), bufs.size(), nullptr, 0, &content->_overlapped, nullptr);

    if ((SOCKET_ERROR == ret) && (WSA_IO_PENDING != WSAGetLastError())) {
        LOG_WARN("IOCP post send event failed! error code: %d", WSAGetLastError());
        return false;
    }

    event->AddType(ET_WRITE);
    LOG_DEBUG("post a new write event");
    return true;
}

bool IOCPEventActions::AddRecvEvent(std::shared_ptr<Event>& event) {
    if (event->GetType() & ET_READ) {
        return false;
    }

    auto sock = event->GetSocket();
    if (!sock) {
        LOG_WARN("socket is already distroyed! event %s", "AddSendEvent");
        return false;
    }

    if (!(event->GetType() & ET_INACTIONS)) {
        if (!AddToIOCP(sock->GetSocket())) {
            return false;
        }
    }

    EventOverlapped* content = (EventOverlapped*)event->GetData();
    auto rw_sock = std::dynamic_pointer_cast<RWSocket>(sock);
    if (!content) {
        content = rw_sock->GetAlocter()->PoolNew<EventOverlapped>();
        event->SetData(content);
    }

    content->_event = (void*)&event;
    content->_event_type = ET_READ;

    auto buffer = rw_sock->GetReadBuffer();
    std::vector<Iovec> bufs;
    buffer->GetFreeMemoryBlock(bufs, __iocp_buff_size);

    int32_t ret = WSARecv(sock->GetSocket(), (LPWSABUF)&(*bufs.begin()), bufs.size(), nullptr, 0, &content->_overlapped, nullptr);

    if ((SOCKET_ERROR == ret) && (WSA_IO_PENDING != WSAGetLastError())) {
        LOG_WARN("IOCP post recv event failed! error code: %d", WSAGetLastError());
        return false;
    }

    event->AddType(ET_READ);
    LOG_DEBUG("post a new read event");
    return true;
}

bool IOCPEventActions::AddAcceptEvent(std::shared_ptr<Event>& event) {
    if (event->GetType() & ET_READ) {
        return false;
    }

    auto sock = event->GetSocket();
    if (!sock) {
        LOG_WARN("socket is already distroyed! event %s", "AddSendEvent");
        return false;
    }

    if (!(event->GetType() & ET_INACTIONS)) {
        if (!AddToIOCP(sock->GetSocket())) {
            return false;
        }
    }

    EventOverlapped* content = (EventOverlapped*)event->GetData();
    if (!content) {
        content = sock->GetAlocter()->PoolNew<EventOverlapped>();
        event->SetData(content);
    }

    content->_event_type = ET_READ;

    uint32_t ret = __AcceptEx((SOCKET)sock->GetSocket(), (SOCKET)event->_client_socket->GetSocket(), &context->_lapped_buffer, context->_wsa_buf.len - ((sizeof(SOCKADDR_IN) + 16) * 2),
        sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &dwBytes, content->_overlapped);

    if (0 == ret) {
        if (WSA_IO_PENDING != WSAGetLastError()) {
            LOG_ERROR("IOCP post accept failed! error code:%d", WSAGetLastError());
            return false;
        }
    }
    LOG_DEBUG("post a new accept event");
    return true;
}

bool IOCPEventActions::AddConnection(std::shared_ptr<Event>& event, Address& address) {

}

bool IOCPEventActions::AddDisconnection(std::shared_ptr<Event>& event) {

}

bool IOCPEventActions::DelEvent(std::shared_ptr<Event>& event) {

}

void IOCPEventActions::ProcessEvent(int32_t wait_ms) {

}

void IOCPEventActions::Wakeup() {
    PostQueuedCompletionStatus(_iocp_handler, 0, INC_WEAK_UP, nullptr);
}

bool IOCPEventActions::AddToIOCP(uint64_t sock) {
    if (CreateIoCompletionPort((HANDLE)sock, _iocp_handler, 0, 0) == NULL) {
        LOG_ERROR("IOCP bind socket to io completion port failed!");
        return false;
    }
    return true;
}

}