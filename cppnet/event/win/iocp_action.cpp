#include <MSWSock.h>
#include <winsock2.h>

#include "expend_func.h"
#include "iocp_action.h"
#include "common/log/log.h"
#include "cppnet/cppnet_config.h"
#include "cppnet/socket/rw_socket.h"
#include "common/buffer/buffer_queue.h"
#include "cppnet/socket/connect_socket.h"
#include "cppnet/event/event_interface.h"

#pragma comment(lib,"ws2_32.lib")

namespace cppnet {

std::shared_ptr<EventActions> MakeEventActions() {
    return std::make_shared<IOCPEventActions>();
}

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
        content->_event = (void*)&event;
        event->SetData(content);
    }
    
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
        content->_event = (void*)&event;
        event->SetData(content);
    }

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
    if (event->GetType() & ET_ACCEPT) {
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
        content->_event = (void*)&event;
        event->SetData(content);
    }

    content->_event_type = ET_ACCEPT;

    uint32_t ret = AcceptEx((SOCKET)sock->GetSocket(), (SOCKET)event->_client_socket->GetSocket(), &context->_lapped_buffer, context->_wsa_buf.len - ((sizeof(SOCKADDR_IN) + 16) * 2),
        sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &dwBytes, content->_overlapped);

    if (0 == ret) {
        if (WSA_IO_PENDING != WSAGetLastError()) {
            LOG_ERROR("IOCP post accept failed! error code:%d", WSAGetLastError());
            return false;
        }
    }

    event->AddType(ET_ACCEPT);
    LOG_DEBUG("post a new accept event");
    return true;
}

bool IOCPEventActions::AddConnection(std::shared_ptr<Event>& event, Address& address) {
    if (event->GetType() & ET_CONNECT) {
        return false;
    }

    auto sock = event->GetSocket();
    if (!sock) {
        LOG_WARN("socket is already distroyed! event %s", "AddSendEvent");
        return false;
    }

    EventOverlapped* content = (EventOverlapped*)event->GetData();
    if (!content) {
        content = sock->GetAlocter()->PoolNew<EventOverlapped>();
        content->_event = (void*)&event;
        event->SetData(content);
    }

    content->_event_type = ET_CONNECT;

    SOCKADDR_IN addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(address.GetPort());
    addr.sin_addr.S_un.S_addr = inet_addr(address.GetIp().c_str());

    /*SOCKADDR_IN local;
    local.sin_family = AF_INET;
    local.sin_port = htons(0);
    local.sin_addr.S_un.S_addr = INADDR_ANY;

    auto socket_ptr = event->_client_socket.Lock();
    if (SOCKET_ERROR == bind(socket_ptr->GetSocket(), (sockaddr*)&local, sizeof(local))) {
        base::LOG_FATAL("bind local host failed! error code: %d", WSAGetLastError());
    }*/

    int32_t ret = ConnectEx(sock->GetSocket(), (sockaddr*)&addr, sizeof(addr), nullptr, 0, nullptr, &content->_overlapped);

    if ((SOCKET_ERROR == ret) && (WSA_IO_PENDING != WSAGetLastError())) {
        LOG_FATAL("IOCP post connect event failed! error code: %d", WSAGetLastError());
        return false;
    }
    LOG_DEBUG("post a new connect event");
    event->AddType(ET_CONNECT);
    return true;
}

bool IOCPEventActions::AddDisconnection(std::shared_ptr<Event>& event) {
    if (event->GetType() & ET_DISCONNECT) {
        return false;
    }

    auto sock = event->GetSocket();
    if (!sock) {
        LOG_WARN("socket is already distroyed! event %s", "AddSendEvent");
        return false;
    }

    EventOverlapped* content = (EventOverlapped*)event->GetData();
    if (!content) {
        content = sock->GetAlocter()->PoolNew<EventOverlapped>();
        content->_event = (void*)&event;
        event->SetData(content);
    }

    content->_event_type = ET_DISCONNECT;

    int32_t ret = DisconnectionEx(sock->GetSocket(), &content->_overlapped, TF_REUSE_SOCKET, 0);

    if ((SOCKET_ERROR == ret) && (WSA_IO_PENDING != WSAGetLastError())) {
        LOG_FATAL("IOCP post disconnect event failed! error code: %d", WSAGetLastError());
        return false;
    }
    LOG_DEBUG("post a new disconnect event");
    event->AddType(ET_DISCONNECT); 
    return true;
}

bool IOCPEventActions::DelEvent(std::shared_ptr<Event>& event) {
    auto sock = event->GetSocket();
    if (!sock) {
        LOG_WARN("socket is already distroyed! event %s", "AddSendEvent");
        return false;
    }

    EventOverlapped* content = (EventOverlapped*)event->GetData();
    if (!content) {
        return true;
    }

    CancelIoEx((HANDLE)sock->GetSocket(), &content->_overlapped);
    return true;
}

void IOCPEventActions::ProcessEvent(int32_t wait_ms) {
    DWORD               bytes_transfered = 0;
    EventOverlapped     *context = nullptr;
    OVERLAPPED          *over_lapped = nullptr;
    
    int32_t ret = GetQueuedCompletionStatus(_iocp_handler, &bytes_transfered, PULONG_PTR(&context),
            &over_lapped, wait_time);

    if (if ((PULONG_PTR)context == (PULONG_PTR)INC_WEAK_UP)) {
        return;
    }
    
    DWORD dw_err = NO_ERROR;
    if (ret != 0) {
        dw_err = GetLastError();
    }

    if (NO_ERROR == dw_err || 
        ERROR_NETNAME_DELETED == dw_err || 
        ERROR_IO_PENDING == dw_err) {
    
        if (over_lapped) {
            context = CONTAINING_RECORD(over_lapped, EventOverlapped, _overlapped);
            LOG_DEBUG("Get a new event : %d", context->_event_type);
            DoEvent(context, bytes_transfered);
        }

    } else if (ERROR_CONNECTION_REFUSED == dw_err || 
               ERROR_SEM_TIMEOUT == dw_err || 
               WSAENOTCONN == dw_err || 
               ERROR_OPERATION_ABORTED == dw_err) {
    
            if (over_lapped) {
                context = CONTAINING_RECORD(over_lapped, EventOverlapped, _overlapped);
                LOG_DEBUG("Get a new event : %d", context->_event_type);
                context->_event_flag_set |= ERR_CONNECT_CLOSE;
                DoEvent(context, bytes_transfered);
            }

    } else {
        LOG_ERROR("IOCP GetQueuedCompletionStatus return error : %d", dw_err);
    }
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

void IOCPEventActions::DoEvent(EventOverlapped *context, uint32_t bytes) {
    std::shared_ptr<Socket> sock;
    std::shared_ptr<Event> event;

    event = *(std::shared_ptr<Event>*)context->_event;
    sock = event->GetSocket();
    if (!sock) {
        LOG_ERROR("socket point is already destory");
        return;
    }

    switch (context->_event_type)
    {
    case ET_ACCEPT: {
        std::shared_ptr<ConnectSocket> connect_sock = std::dynamic_pointer_cast<ConnectSocket>(sock);
        connect_sock->OnAccept());
        event->RemoveType(ET_ACCEPT);
        break;
    }
    case ET_READ: {
        std::shared_ptr<RWSocket> rw_socket = std::dynamic_pointer_cast<RWSocket>(sock);
        rw_socket->OnRead(bytes);
        event->RemoveType(ET_READ);
        break;
    }
    case ET_WRITE: {
        std::shared_ptr<RWSocket> rw_socket = std::dynamic_pointer_cast<RWSocket>(sock);
        rw_socket->OnWrite(bytes);
        event->RemoveType(ET_WRITE);
        break;
    }
    case ET_CONNECT: {
        std::shared_ptr<RWSocket> rw_socket = std::dynamic_pointer_cast<RWSocket>(sock);
        rw_socket->OnConnect(CEC_SUCCESS);
        event->RemoveType(ET_CONNECT);
        break;
    }
    case ET_DISCONNECT: {
        std::shared_ptr<RWSocket> rw_socket = std::dynamic_pointer_cast<RWSocket>(sock);
        rw_socket->OnDisConnect(CEC_CLOSED);
        event->RemoveType(ET_DISCONNECT);
        break;
    }
    
    default:
        LOG_ERROR("invalid event type. type:%d", context->_event_type);
        break;
    }
    context->_event_type = 0;
}

}