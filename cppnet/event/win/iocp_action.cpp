// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#include "expend_func.h"
#include "iocp_action.h"

#include "include/cppnet_type.h"
#include "cppnet/cppnet_config.h"
#include "cppnet/event/win/accept_event.h"
#include "cppnet/socket/win/win_rw_socket.h"
#include "cppnet/socket/win/win_connect_socket.h"

#include "common/log/log.h"
#include "common/os/convert.h"
#include "common/network/socket.h"
#include "common/buffer/buffer_queue.h"

namespace cppnet {

ThreadSafeUnorderedMap<uint64_t, std::shared_ptr<Socket>> IOCPEventActions::__connecting_socket_map;

std::shared_ptr<EventActions> MakeEventActions() {
    return std::make_shared<IOCPEventActions>();
}

IOCPEventActions::IOCPEventActions():
    _iocp_handler(nullptr) {

}

IOCPEventActions::~IOCPEventActions() {

}

bool IOCPEventActions::Init(uint32_t thread_num) {
    WinSockInit();
    //tell IOCP thread num
    _iocp_handler = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, thread_num);
    if (_iocp_handler == INVALID_HANDLE_VALUE) {
        LOG_ERROR("IOCP create IO completion port failed!");
        return false;
    }
    return true;
}

bool IOCPEventActions::Dealloc() {
    Wakeup();
    return true;
}

bool IOCPEventActions::AddSendEvent(Event* event) {
    if (event->GetType() & ET_WRITE || event->GetType() & ET_DISCONNECT) {
        LOG_WARN_S << "repeat send event";
        return false;
    }

    auto sock = event->GetSocket();
    if (!sock) {
        LOG_WARN("socket is already destroyed! event %s", "AddSendEvent");
        return false;
    }
    auto rw_sock = std::dynamic_pointer_cast<WinRWSocket>(sock);
    if (rw_sock->IsShutdown()) {
        LOG_WARN_S << "socket is shutdown when send";
        rw_sock->OnDisConnect(event, CEC_CONNECT_BREAK);
        return false;
    }

    EventOverlapped* context = (EventOverlapped*)event->GetData();
    if (!context) {
        context = rw_sock->GetAlloter()->PoolNew<EventOverlapped>();
        context->_event = (void*)event;
        event->SetData(context);
    }

    context->_event_type = ET_WRITE;

    auto buffer = event->GetBuffer();
    std::vector<Iovec> bufs;
    buffer->GetUseMemoryBlock(bufs, __iocp_buff_size);

    DWORD dwFlags = 0;
    int32_t ret = WSASend((SOCKET)sock->GetSocket(), (LPWSABUF) & (*bufs.begin()), (DWORD)bufs.size(), nullptr, dwFlags, &context->_overlapped, nullptr);

    if ((SOCKET_ERROR == ret) && (WSA_IO_PENDING != WSAGetLastError())) {
        LOG_WARN("IOCP post send event failed! error code:%d, info:%s", WSAGetLastError(), ErrnoInfo(WSAGetLastError()));
        rw_sock->SetShutdown();
        rw_sock->OnDisConnect(event, CEC_CONNECT_BREAK);
        return false;
    }

    // send some data immediately
    event->AddType(ET_WRITE);
    LOG_DEBUG("post a new write event");
    return true;
}

bool IOCPEventActions::AddRecvEvent(Event* event) {
    if (event->GetType() & ET_READ || event->GetType() & ET_DISCONNECT) {
        LOG_WARN_S << "repeat recv event";
        return false;
    }

    auto sock = event->GetSocket();
    if (!sock) {
        LOG_WARN("socket is already destroyed! event %s", "AddSendEvent");
        return false;
    }
    auto rw_sock = std::dynamic_pointer_cast<WinRWSocket>(sock);
    if (rw_sock->IsShutdown()) {
        LOG_WARN_S << "socket is shutdown when recv";
        rw_sock->OnDisConnect(event, CEC_CONNECT_BREAK);
        return false;
    }

    EventOverlapped* context = (EventOverlapped*)event->GetData();

    if (!context) {
        context = rw_sock->GetAlloter()->PoolNew<EventOverlapped>();
        context->_event = (void*)event;
        event->SetData(context);
    }
    context->_event_type = ET_READ;

    auto buffer = event->GetBuffer();
    std::vector<Iovec> bufs;
    buffer->GetFreeMemoryBlock(bufs, __iocp_buff_size);

    DWORD dwFlags = 0;
    int32_t ret = WSARecv((SOCKET)sock->GetSocket(), (LPWSABUF) & (*bufs.begin()), (DWORD)bufs.size(), nullptr, &dwFlags, &context->_overlapped, nullptr);

    if ((SOCKET_ERROR == ret) && (WSA_IO_PENDING != WSAGetLastError())) {
        LOG_WARN("IOCP post recv event failed! error code: %d, info:%s", WSAGetLastError(), ErrnoInfo(WSAGetLastError()));
        rw_sock->SetShutdown();
        rw_sock->OnDisConnect(event, CEC_CONNECT_BREAK);
        return false;
    }
    event->AddType(ET_READ);
    LOG_DEBUG("post a new read event");
    return true;
}

bool IOCPEventActions::AddAcceptEvent(Event* event) {
    if (event->GetType() & ET_ACCEPT) {
        LOG_WARN_S << "repeat accept event";
        return false;
    }

    auto sock = event->GetSocket();
    if (!sock) {
        LOG_WARN("socket is already destroyed! event %s", "AddWinAcceptEvent");
        return false;
    }

    auto accept_sock = std::dynamic_pointer_cast<WinConnectSocket>(sock);
    auto accept_event = dynamic_cast<WinAcceptEvent*>(event);

    EventOverlapped* context = (EventOverlapped*)event->GetData();
    if (!context) {
        context = new EventOverlapped();
        context->_event = (void*)event;
        event->SetData(context);
    }
    context->_event_type = ET_ACCEPT;

    DWORD dwBytes = 0;
    uint32_t ret = AcceptEx((SOCKET)sock->GetSocket(), (SOCKET)accept_event->GetClientSocket(), accept_event->GetBuf(), __iocp_buff_size - ((sizeof(SOCKADDR_STORAGE) + 16) * 2),
        sizeof(SOCKADDR_STORAGE) + 16, sizeof(SOCKADDR_STORAGE) + 16, &dwBytes, &context->_overlapped);

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

bool IOCPEventActions::AddConnection(Event* event, Address& address) {
    if (event->GetType() & ET_CONNECT) {
        LOG_WARN_S << "repeat connect event";
        return false;
    }

    auto sock = event->GetSocket();
    if (!sock) {
        LOG_WARN("socket is already destroyed! event %s", "AddSendEvent");
        return false;
    }

    EventOverlapped* context = (EventOverlapped*)event->GetData();
    if (!context) {
        auto rw_sock = std::dynamic_pointer_cast<WinRWSocket>(sock);
        context = rw_sock->GetAlloter()->PoolNew<EventOverlapped>();
        context->_event = (void*)event;
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

    context->_event_type = ET_CONNECT;

    DWORD dwBytes = 0;
    int32_t ret = 0;
    if (address.GetType() == AT_IPV4) {
        SOCKADDR_IN addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(address.GetAddrPort());
        addr.sin_addr.S_un.S_addr = inet_addr(address.GetIp().c_str());
        ret = ConnectEx((SOCKET)sock->GetSocket(), (sockaddr*)&addr, sizeof(addr), nullptr, 0, &dwBytes, &context->_overlapped);

    } else {
        SOCKADDR_IN6 addr;
        addr.sin6_flowinfo = 0;
        addr.sin6_scope_id = 0;
        addr.sin6_family = AF_INET6;
        addr.sin6_port = htons(address.GetAddrPort());
        inet_pton(AF_INET6, address.GetIp().c_str(), &addr.sin6_addr);
        ret = ConnectEx((SOCKET)sock->GetSocket(), (sockaddr*)&addr, sizeof(addr), nullptr, 0, &dwBytes, &context->_overlapped);
    }

    setsockopt((SOCKET)sock->GetSocket(), SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0);

    auto rw_sock = std::dynamic_pointer_cast<RWSocket>(sock);
    if (ret) {
        rw_sock->OnConnect(event, CEC_SUCCESS);
        return true;
    }

    int32_t err = WSAGetLastError();
    if (WSA_IO_PENDING == err || ERROR_SUCCESS == err) {
        if (CheckConnect(rw_sock->GetSocket())) {
            rw_sock->OnConnect(event, CEC_SUCCESS);
            return true;
        }
    }

    __connecting_socket_map[sock->GetSocket()] = sock;
    return true;
}

bool IOCPEventActions::AddDisconnection(Event* event) {
    if (event->GetType() & ET_DISCONNECT) {
        LOG_WARN_S << "repeat disconnect event";
        return false;
    }

    auto sock = event->GetSocket();
    if (!sock) {
        LOG_WARN("socket is already destroyed! event %s", "AddSendEvent");
        return false;
    }
    auto rw_sock = std::dynamic_pointer_cast<WinRWSocket>(sock);
    if (rw_sock->IsShutdown()) {
        LOG_WARN_S << "socket is shutdown when disconnect";
        rw_sock->OnDisConnect(event, CEC_CLOSED);
        return false;
    }
    rw_sock->SetShutdown();

    EventOverlapped* context = (EventOverlapped*)event->GetData();
    if (!context) {
        auto rw_sock = std::dynamic_pointer_cast<WinRWSocket>(sock);
        context = rw_sock->GetAlloter()->PoolNew<EventOverlapped>();
        context->_event = (void*)event;
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
    return true;
}

bool IOCPEventActions::DelEvent(Event* event) {
    auto sock = event->GetSocket();
    if (!sock) {
        LOG_WARN("socket is already destroyed! event %s", "AddSendEvent");
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
        // do nothing

    } else if (ERROR_SEM_TIMEOUT == dw_err || 
             //WSAENOTCONN == dw_err       || 
               WSAECONNABORTED == dw_err   ||
               WSAENOTSOCK == dw_err       || 
               ERROR_OPERATION_ABORTED == dw_err) {

        // why ConnectEx get WSAENOTCONN? 
        // fucking 10057, shouldn't care?
        //if (!((context->_event_type == ET_CONNECT || context->_event_type == ET_DISCONNECT) &&
        //    WSAENOTCONN == dw_err)) {
            context->_event_type = INC_CONNECTION_BREAK;
        //}

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
        LOG_ERROR("IOCP bind socket to IO completion port failed!");
        return false;
    }
    return true;
}

void IOCPEventActions::DoEvent(EventOverlapped *context, uint32_t bytes) {
    std::shared_ptr<Socket> sock;
    Event* event = (Event*)context->_event;
    if (!event) {
        LOG_ERROR("event point is already destroy");
        return;
    }
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
        auto accpet_event = dynamic_cast<WinAcceptEvent*>(event);
        accpet_event->SetBufOffset(bytes);
        std::shared_ptr<WinConnectSocket> connect_sock = std::dynamic_pointer_cast<WinConnectSocket>(sock);
        connect_sock->OnAccept(accpet_event);
        break;
    }
    case ET_READ: {
        std::shared_ptr<WinRWSocket> rw_socket = std::dynamic_pointer_cast<WinRWSocket>(sock);
        if (bytes == 0) {
            rw_socket->SetShutdown();
            rw_socket->OnDisConnect(event, CEC_CLOSED);
            
        } else {
            rw_socket->OnRead(event, bytes);
        }
        break;
    }
    case ET_WRITE: {
        std::shared_ptr<WinRWSocket> rw_socket = std::dynamic_pointer_cast<WinRWSocket>(sock);
        if (bytes == 0) {
            rw_socket->SetShutdown();
            rw_socket->OnDisConnect(event, CEC_CLOSED);
           
        } else {
            rw_socket->OnWrite(event, bytes);
        }
        
        break;
    }
    case ET_CONNECT: {
        std::shared_ptr<RWSocket> rw_socket = std::dynamic_pointer_cast<RWSocket>(sock);

        if (CheckConnect(rw_socket->GetSocket())) {
            rw_socket->OnConnect(event, CEC_SUCCESS);

        } else {
            rw_socket->OnConnect(event, CEC_CONNECT_REFUSE);
        }
        __connecting_socket_map.Erase(rw_socket->GetSocket());
        break;
    }
    case INC_CONNECTION_CLOSE:
    case ET_DISCONNECT: {
        std::shared_ptr<WinRWSocket> rw_socket = std::dynamic_pointer_cast<WinRWSocket>(sock);
        if (rw_socket) {
            rw_socket->SetShutdown();
            rw_socket->OnDisConnect(event, CEC_CLOSED);

        } else {
            LOG_WARN_S << "disconnect empty socket";
        }
        break;
    }
    case INC_CONNECTION_BREAK: {
        std::shared_ptr<WinRWSocket> rw_socket = std::dynamic_pointer_cast<WinRWSocket>(sock);
        if (rw_socket) {
            rw_socket->SetShutdown();
            rw_socket->OnDisConnect(event, CEC_CONNECT_BREAK);
        }  else {
            LOG_WARN_S << "connect break empty socket";
        }
        break;
    }
    case INC_CONNECTION_REFUSE: {
        std::shared_ptr<RWSocket> rw_socket = std::dynamic_pointer_cast<RWSocket>(sock);
        rw_socket->OnConnect(event, CEC_CONNECT_REFUSE);
        break;
    }
    default:
        LOG_ERROR("invalid event type. type:%d", context->_event_type);
        break;
    }
}

}