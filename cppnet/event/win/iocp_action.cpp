// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#include <WS2tcpip.h>
#include "expend_func.h"
#include "iocp_action.h"

#include "cppnet/cppnet_config.h"
#include "cppnet/socket/rw_socket.h"
#include "cppnet/event/win/rw_event.h"
#include "cppnet/event/win/accept_event.h"
#include "cppnet/socket/win/win_connect_socket.h"

#include "common/log/log.h"
#include "common/os/convert.h"
#include "common/network/socket.h"
#include "common/buffer/buffer_queue.h"


namespace cppnet {

std::shared_ptr<EventActions> MakeEventActions() {
    return std::make_shared<IOCPEventActions>();
}

enum IOCP_NOTIFY_CODE {
    INC_WEAK_UP           = 0xAAAAFFFF,
    INC_CONNECTION_BREAK  = 0x100,
    INC_CONNECTION_REFUSE = 0x200,
    INC_CONNECTION_CLOSE  = 0x400,
};

struct EventOverlapped {
    OVERLAPPED    _overlapped;
    uint32_t      _event_type;
    void*         _event;

    EventOverlapped(): 
        _event_type(0),
        _event(nullptr){
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
    WinSockInit();
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
    if (event->GetType() & ET_WRITE || event->GetType() & ET_DISCONNECT) {
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
        event->AddType(ET_INACTIONS);
    }
    auto rw_event = std::dynamic_pointer_cast<RWEvent>(event);
    EventOverlapped* context = (EventOverlapped*)rw_event->GetExData();
    auto rw_sock = std::dynamic_pointer_cast<RWSocket>(sock);
    if (!context) {
        context = rw_sock->GetAlloter()->PoolNew<EventOverlapped>();
        context->_event = (void*)&event;
        rw_event->SetExData(context);
    }
    
    context->_event_type = ET_WRITE;

    auto buffer = rw_sock->GetWriteBuffer();
    std::vector<Iovec> bufs;
    buffer->GetUseMemoryBlock(bufs, __iocp_buff_size);

    DWORD dwFlags = 0;
    int32_t ret = WSASend((SOCKET)sock->GetSocket(), (LPWSABUF)&(*bufs.begin()), (DWORD)bufs.size(), nullptr, dwFlags, &context->_overlapped, nullptr);

    if ((SOCKET_ERROR == ret) && (WSA_IO_PENDING != WSAGetLastError())) {
        LOG_WARN("IOCP post send event failed! error code:%d, info:%s", WSAGetLastError(), ErrnoInfo(WSAGetLastError()));
        rw_sock->OnDisConnect(CEC_CLOSED);
        DelEvent(event);
        return false;
    }

    // send some data immediately
    event->AddType(ET_WRITE);
    LOG_DEBUG("post a new write event");
    return true;
}

bool IOCPEventActions::AddRecvEvent(std::shared_ptr<Event>& event) {
    if (event->GetType() & ET_READ || event->GetType() & ET_DISCONNECT) {
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
        event->AddType(ET_INACTIONS);
    }

    EventOverlapped* context = (EventOverlapped*)event->GetData();
    auto rw_sock = std::dynamic_pointer_cast<RWSocket>(sock);
    if (!context) {
        context = rw_sock->GetAlloter()->PoolNew<EventOverlapped>();
        context->_event = (void*)&event;
        event->SetData(context);
    }
    context->_event_type = ET_READ;

    auto buffer = rw_sock->GetReadBuffer();
    std::vector<Iovec> bufs;
    buffer->GetFreeMemoryBlock(bufs, __iocp_buff_size);

    DWORD dwFlags = 0;
    int32_t ret = WSARecv((SOCKET)sock->GetSocket(), (LPWSABUF)&(*bufs.begin()), (DWORD)bufs.size(), nullptr, &dwFlags, &context->_overlapped, nullptr);

    if ((SOCKET_ERROR == ret) && (WSA_IO_PENDING != WSAGetLastError())) {
        LOG_WARN("IOCP post recv event failed! error code: %d, info:%s", WSAGetLastError(), ErrnoInfo(WSAGetLastError()));
        rw_sock->OnDisConnect(CEC_CLOSED);
        DelEvent(event);
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
        LOG_WARN("socket is already distroyed! event %s", "AddAcceptEvent");
        return false;
    }

    auto accept_sock = std::dynamic_pointer_cast<WinConnectSocket>(sock);
    if (!accept_sock->GetInActions()) {
        if (!AddToIOCP(sock->GetSocket())) {
            return false;
        }
        accept_sock->SetInActions(true);
    }

    auto accept_event = std::dynamic_pointer_cast<AcceptEvent>(event);

    EventOverlapped* context = (EventOverlapped*)event->GetData();
    if (!context) {
        context = new EventOverlapped();
        context->_event = (void*)&event;
        event->SetData(context);
    }
    context->_event_type = ET_ACCEPT;
   
    DWORD dwBytes = 0;
    uint32_t ret = AcceptEx((SOCKET)sock->GetSocket(), (SOCKET)accept_event->GetClientSocket(), accept_event->GetBuf(), __iocp_buff_size - ((sizeof(SOCKADDR_STORAGE) + 16) * 2),
        sizeof(SOCKADDR_STORAGE) + 16, sizeof(SOCKADDR_STORAGE) + 16, &dwBytes, &context->_overlapped);

    if (0 == ret) {
        if (WSA_IO_PENDING != WSAGetLastError()) {
            LOG_ERROR("IOCP post accept failed! error code:%d", WSAGetLastError());
            DelEvent(event);
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

    EventOverlapped* context = (EventOverlapped*)event->GetData();
    if (!context) {
        context = sock->GetAlloter()->PoolNew<EventOverlapped>();
        context->_event = (void*)&event;
        event->SetData(context);
    }

    
	if (address.GetType() == AT_IPV4) {
		SOCKADDR_IN local;
		local.sin_family = AF_INET;
		local.sin_port = htons(0);
		local.sin_addr.S_un.S_addr = INADDR_ANY;
		if (bind(sock->GetSocket(), (sockaddr*)&local, sizeof(local)) != 0) {
			LOG_FATAL("bind local host failed! error code:%d, info:%s", WSAGetLastError(), ErrnoInfo(WSAGetLastError()));
		}

	} else {
		SOCKADDR_IN6 local;
        local.sin6_flowinfo = 0;
        local.sin6_scope_id = 0;
        local.sin6_family = AF_INET6;
        local.sin6_port = 0;
        local.sin6_addr = in6addr_any;
		if (bind(sock->GetSocket(), (sockaddr*)&local, sizeof(local)) != 0) {
			LOG_FATAL("bind local host failed! error code:%d, info:%s", WSAGetLastError(), ErrnoInfo(WSAGetLastError()));
		}
	}

    DWORD dwBytes = 0;
    int32_t ret = -1;
    if (address.GetType() == AT_IPV4) {
		SOCKADDR_IN addr;
		addr.sin_family = AF_INET6;
		addr.sin_port = htons(address.GetAddrPort());
		addr.sin_addr.S_un.S_addr = inet_addr(address.GetIp().c_str());
        ConnectEx((SOCKET)sock->GetSocket(), (sockaddr*)&addr, sizeof(addr), nullptr, 0, &dwBytes, &context->_overlapped);

    } else {
		SOCKADDR_IN6 addr;
		addr.sin6_flowinfo = 0;
		addr.sin6_scope_id = 0;
		addr.sin6_family = AF_INET6;
		addr.sin6_port = htons(address.GetAddrPort());
		inet_pton(AF_INET6, address.GetIp().c_str(), &addr.sin6_addr);
        ConnectEx((SOCKET)sock->GetSocket(), (sockaddr*)&addr, sizeof(addr), nullptr, 0, &dwBytes, &context->_overlapped);
    }
    
    setsockopt((SOCKET)sock->GetSocket(), SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0);

    auto rw_sock = std::dynamic_pointer_cast<RWSocket>(sock);
    if (ret) {
        rw_sock->OnConnect(CEC_SUCCESS);
        return true;
    }

    int32_t err = WSAGetLastError();
    if (WSA_IO_PENDING == err || ERROR_SUCCESS == err) {
        if (CheckConnect(rw_sock->GetSocket())) {
            rw_sock->OnConnect(CEC_SUCCESS);
            return true;
        }
    }
    context->_event_type = ET_CONNECT;
    DoEvent(context, 0);

    return false;
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

    EventOverlapped* context = (EventOverlapped*)event->GetData();
    if (!context) {
        context = sock->GetAlloter()->PoolNew<EventOverlapped>();
        context->_event = (void*)&event;
        event->SetData(context);
    }

    context->_event_type = ET_DISCONNECT;

    int32_t ret = DisconnectionEx((SOCKET)sock->GetSocket(), &context->_overlapped, TF_REUSE_SOCKET, 0);

    if ((SOCKET_ERROR == ret) && (WSA_IO_PENDING != WSAGetLastError())) {
        LOG_FATAL("IOCP post disconnect event failed! error code: %d", WSAGetLastError());
        return false;
    }
    LOG_DEBUG("post a new disconnect event");

    event->AddType(ET_DISCONNECT);
    DelEvent(event);
    auto rw_sock = std::dynamic_pointer_cast<RWSocket>(sock);
    rw_sock->OnDisConnect(CEC_CLOSED);
    return true;
}

bool IOCPEventActions::DelEvent(std::shared_ptr<Event>& event) {
    auto sock = event->GetSocket();
    if (!sock) {
        LOG_WARN("socket is already distroyed! event %s", "AddSendEvent");
        return false;
    }

    EventOverlapped* context = (EventOverlapped*)event->GetData();
    if (!context) {
        return true;
    }

    CancelIoEx((HANDLE)sock->GetSocket(), &context->_overlapped);
    return true;
}

void IOCPEventActions::ProcessEvent(int32_t wait_ms) {
    DWORD               bytes_transfered = 0;
    EventOverlapped     *context = nullptr;
    OVERLAPPED          *over_lapped = nullptr;
    
    int32_t ret = GetQueuedCompletionStatus(_iocp_handler, &bytes_transfered, PULONG_PTR(&context), &over_lapped, wait_ms);

    if (((PULONG_PTR)context == (PULONG_PTR)INC_WEAK_UP)) {
        return;
    }
    
    DWORD dw_err = NO_ERROR;
    if (ret != 0) {
        dw_err = GetLastError();
    }

	if (over_lapped) {
		context = CONTAINING_RECORD(over_lapped, EventOverlapped, _overlapped);
		LOG_DEBUG("Get a new event: %s", TypeString((EventType)context->_event_type));

    } else {
        LOG_ERROR("Get a new event but lapped is null");
        return;
    }

    if (NO_ERROR == dw_err ||
        ERROR_IO_PENDING == dw_err) {

    } else if (ERROR_SEM_TIMEOUT == dw_err || 
               WSAENOTCONN == dw_err || 
               ERROR_OPERATION_ABORTED == dw_err) {
        context->_event_type = INC_CONNECTION_BREAK;

    } else if (ERROR_NETNAME_DELETED == dw_err) {
        context->_event_type = INC_CONNECTION_CLOSE;

    } else if (ERROR_CONNECTION_REFUSED == dw_err) {
        context->_event_type = INC_CONNECTION_REFUSE;

    } else {
        LOG_INFO("IOCP GetQueuedCompletionStatus return error:%d, info:%s", dw_err, ErrnoInfo(dw_err));
    }
    DoEvent(context, bytes_transfered);
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
    if (context->_event_type > INC_CONNECTION_CLOSE ||
        context->_event_type < ET_READ) {
        return;
    }
    std::shared_ptr<Socket> sock;
    std::shared_ptr<Event> event;

    event = *(std::shared_ptr<Event>*)context->_event;
    sock = event->GetSocket();
    if (!sock) {
        LOG_ERROR("socket point is already destroy");
        return;
    }

    switch (context->_event_type)
    {
    case ET_ACCEPT: {
        context->_event_type = 0;
        event->RemoveType(ET_ACCEPT);
        auto accpet_event = std::dynamic_pointer_cast<AcceptEvent>(event);
        accpet_event->SetBufOffset(bytes);
        std::shared_ptr<WinConnectSocket> connect_sock = std::dynamic_pointer_cast<WinConnectSocket>(sock);
        connect_sock->OnAccept(accpet_event);
        break;
    }
    case ET_READ: {
        context->_event_type = 0;
        event->RemoveType(ET_READ);
        std::shared_ptr<RWSocket> rw_socket = std::dynamic_pointer_cast<RWSocket>(sock);
        if (bytes == 0) {
            rw_socket->OnDisConnect(CEC_CLOSED);
        } else {
            rw_socket->OnRead(bytes);
        }
        break;
    }
    case ET_WRITE: {
        context->_event_type = 0;
        event->RemoveType(ET_WRITE);
        std::shared_ptr<RWSocket> rw_socket = std::dynamic_pointer_cast<RWSocket>(sock);
        if (bytes == 0) {
            rw_socket->OnDisConnect(CEC_CLOSED);
        } else {
            rw_socket->OnWrite(bytes);
        }
        
        break;
    }
    case ET_CONNECT: {
        context->_event_type = 0;
        event->RemoveType(ET_CONNECT);
        std::shared_ptr<RWSocket> rw_socket = std::dynamic_pointer_cast<RWSocket>(sock);
        rw_socket->OnConnect(CEC_SUCCESS);
        break;
    }
    case INC_CONNECTION_CLOSE:
    case ET_DISCONNECT: {
        context->_event_type = 0;
        event->RemoveType(ET_DISCONNECT);
        std::shared_ptr<RWSocket> rw_socket = std::dynamic_pointer_cast<RWSocket>(sock);
        rw_socket->OnDisConnect(CEC_CLOSED);
        break;
    }
    case INC_CONNECTION_BREAK: {
        context->_event_type = 0;
        event->RemoveType(ET_DISCONNECT);
        std::shared_ptr<RWSocket> rw_socket = std::dynamic_pointer_cast<RWSocket>(sock);
        if (rw_socket) {
            rw_socket->OnDisConnect(CEC_CONNECT_BREAK);
        }
        break;
    }
    case INC_CONNECTION_REFUSE: {
        context->_event_type = 0;
        event->RemoveType(ET_CONNECT);
        std::shared_ptr<RWSocket> rw_socket = std::dynamic_pointer_cast<RWSocket>(sock);
        rw_socket->OnConnect(CEC_CONNECT_REFUSE);
        break;
    }
    default:
        LOG_ERROR("invalid event type. type:%d", context->_event_type);
        break;
    }
}

}