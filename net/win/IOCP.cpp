#ifndef __linux__
#include "IOCP.h"
#include "Log.h"
#include "OSInfo.h"
#include "EventHandler.h"
#include "Buffer.h"
#include "WinExpendFunc.h"
#include "Timer.h"
#include "CppNetImpl.h"

using namespace cppnet;

enum STATE_CODE {
    EXIT_IOCP    = 0xFFFFFFFF,
    WEAK_UP_IOCP = 0xAAAAFFFF,
};

CIOCP::CIOCP() : _is_inited(false), _run(true) {

}

CIOCP::~CIOCP() {

}

bool CIOCP::Init(uint32_t thread_num) {
    if (thread_num == 0) {
        thread_num = CCppNetImpl::Instance().GetThreadNum();
    }
    //tell iocp the must thread num
    _iocp_handler = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, thread_num);
    if (_iocp_handler == INVALID_HANDLE_VALUE) {
        base::LOG_FATAL("IOCP create io completion port failed!");
        return false;
    }
    _is_inited = true;
    return true;
}

bool CIOCP::Dealloc() {
    _run = false;
    PostQueuedCompletionStatus(_iocp_handler, 0, EXIT_IOCP, nullptr);
    return true;
}

uint64_t CIOCP::AddTimerEvent(unsigned int interval, const std::function<void(void*)>& call_back, void* param, bool always) {
    return _timer.AddTimer(interval, call_back, param, always);
}

bool CIOCP::RemoveTimerEvent(uint64_t timer_id) {
    return _timer.DelTimer(timer_id);
}

bool CIOCP::AddTimerEvent(unsigned int interval, base::CMemSharePtr<CEventHandler>& event) {
    _timer.AddTimer(interval, event);
    return true;
}

bool CIOCP::AddSendEvent(base::CMemSharePtr<CEventHandler>& event) {
    auto socket_ptr = event->_client_socket.Lock();
    if (socket_ptr && _AddToActions(socket_ptr)) {
        ((EventOverlapped*)event->_data)->_event = &event;
        return _PostSend(event);
    }
    base::LOG_WARN("socket is already destroyed!");
    return false;
}

bool CIOCP::AddRecvEvent(base::CMemSharePtr<CEventHandler>& event) {
    auto socket_ptr = event->_client_socket.Lock();
    if (socket_ptr && _AddToActions(socket_ptr)) {
        ((EventOverlapped*)event->_data)->_event = &event;
        return _PostRecv(event);
    }
    base::LOG_WARN("socket is already destroyed!");
    return false;
}

bool CIOCP::AddAcceptEvent(base::CMemSharePtr<CAcceptEventHandler>& event) {
    if (!event->_accept_socket->IsInActions()) {
        if (CreateIoCompletionPort((HANDLE)(event->_accept_socket->GetSocket()), _iocp_handler, 0, 0) == NULL) {
            base::LOG_ERROR("IOCP bind socket to io completion port failed!");
            return false;
        }
    }
    ((EventOverlapped*)event->_data)->_event = &event;
    event->_accept_socket->SetInActions(true);
    return _PostAccept(event);
}

bool CIOCP::AddConnection(base::CMemSharePtr<CEventHandler>& event, const std::string& ip, short port, const char* buf, int buf_len) {
    auto socket_ptr = event->_client_socket.Lock();
    if (socket_ptr && _AddToActions(socket_ptr)) {
        ((EventOverlapped*)event->_data)->_event = &event;
        return _PostConnection(event, ip, port, buf, buf_len);
    }
    base::LOG_WARN("socket is already destroyed!");
    return false;
}

bool CIOCP::AddDisconnection(base::CMemSharePtr<CEventHandler>& event) {
    auto socket_ptr = event->_client_socket.Lock();
    if (socket_ptr && _AddToActions(socket_ptr)) {
        ((EventOverlapped*)event->_data)->_event = &event;
        return _PostDisconnection(event);
    }
    base::LOG_WARN("socket is already destroyed!");
    return false;
}

bool CIOCP::DelEvent(base::CMemSharePtr<CEventHandler>& event) {
    ((EventOverlapped*)event->_data)->_event = nullptr;
    auto socket_ptr = event->_client_socket.Lock();
    if (socket_ptr) {
        CancelIoEx((HANDLE)socket_ptr->GetSocket(), &((EventOverlapped*)event->_data)->_overlapped);
    }
    return true;
}

void CIOCP::ProcessEvent() {
    DWORD                bytes_transfered = 0;
    EventOverlapped        *socket_context  = nullptr;
    OVERLAPPED          *over_lapped     = nullptr;
    unsigned int        wait_time        = 0;
    std::vector<base::CMemSharePtr<CTimerEvent>> timer_vec;
    while (_run) {
        wait_time = _timer.TimeoutCheck(timer_vec);
        //if there is no timer event. wait until recv something
        if (wait_time == 0 && timer_vec.empty()) {
            wait_time = INFINITE;

        } else {
            wait_time  = wait_time > 0 ? wait_time : 1;
        }

        int res = GetQueuedCompletionStatus(_iocp_handler, &bytes_transfered, PULONG_PTR(&socket_context),
            &over_lapped, wait_time);

        DWORD dw_err = 0;
        if (res) {
            dw_err = NO_ERROR;
            // exit
            if ((PULONG_PTR)socket_context == (PULONG_PTR)EXIT_IOCP){
                break;
            }

        } else {
            dw_err = GetLastError();
        }
    
        // timer out event
        if (dw_err == WAIT_TIMEOUT) {
            if (!timer_vec.empty()) {
                _DoTimeoutEvent(timer_vec);
            }
            _DoTaskList();

        // read some thing
        } else if (ERROR_NETNAME_DELETED == dw_err || NO_ERROR == dw_err || ERROR_IO_PENDING == dw_err) {
            if (over_lapped) {
                socket_context = CONTAINING_RECORD(over_lapped, EventOverlapped, _overlapped);
                base::LOG_DEBUG("Get a new event : %d", socket_context->_event_flag_set);
                _DoEvent(socket_context, bytes_transfered);
            }
            _DoTaskList();

        } else if (ERROR_CONNECTION_REFUSED == dw_err || ERROR_SEM_TIMEOUT == dw_err) {
            if (over_lapped) {
                socket_context = CONTAINING_RECORD(over_lapped, EventOverlapped, _overlapped);
                base::LOG_DEBUG("Get a new event : %d", socket_context->_event_flag_set);
                socket_context->_event_flag_set |= ERR_CONNECT_CLOSE;
                _DoEvent(socket_context, bytes_transfered);
            }
            _DoTaskList();

        } else {
            base::LOG_ERROR("IOCP GetQueuedCompletionStatus return error : %d", dw_err);
            continue;
        }
    }

    if (_is_inited) {
        // only one iocp handle
        static bool once = true;
        if (once) {
            once = false;
            if (CloseHandle(_iocp_handler) == -1) {
                base::LOG_ERROR("IOCP close io completion port failed!");
            }
        }
        
    }
    _is_inited = false;
}

void CIOCP::PostTask(std::function<void(void)>& task) {
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _task_list.push_back(task);
    }
    WakeUp();
}

void CIOCP::WakeUp() {
    PostQueuedCompletionStatus(_iocp_handler, 0, WEAK_UP_IOCP, nullptr);
}

bool CIOCP::_PostRecv(base::CMemSharePtr<CEventHandler>& event) {
    EventOverlapped* context = (EventOverlapped*)event->_data;

    DWORD dwFlags = 0;
    DWORD dwBytes = 0;
    context->Clear();
    context->_event_flag_set = event->_event_flag_set;
    OVERLAPPED *lapped = &context->_overlapped;
    auto socket_ptr = event->_client_socket.Lock();

    int res = WSARecv(socket_ptr->GetSocket(), &context->_wsa_buf, 1, &dwFlags, &dwBytes, lapped, nullptr);

    if ((SOCKET_ERROR == res) && (WSA_IO_PENDING != WSAGetLastError())) {
        base::LOG_WARN("IOCP post recv event failed! error code: %d", WSAGetLastError());
        return false;
    }
    base::LOG_DEBUG("post a new event : %d", context->_event_flag_set);
    return true;
}

bool CIOCP::_PostAccept(base::CMemSharePtr<CAcceptEventHandler>& event) {
    if (!__AcceptEx) {
        base::LOG_ERROR("__AcceptEx function is null!");
        return false;
    }

    EventOverlapped* context = (EventOverlapped*)event->_data;
    context->Clear();
    DWORD dwBytes = 0;
    context->_event_flag_set |= event->_event_flag_set;
    OVERLAPPED *lapped = &context->_overlapped;
    
    int res = __AcceptEx((SOCKET)event->_accept_socket->GetSocket(), (SOCKET)event->_client_socket->GetSocket(), &context->_lapped_buffer, context->_wsa_buf.len - ((sizeof(SOCKADDR_IN) + 16) * 2), 
        sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &dwBytes, lapped);

    if (FALSE == res) {
        if (WSA_IO_PENDING != WSAGetLastError()) {
            base::LOG_ERROR("IOCP post accept failed! error code:%d", WSAGetLastError());
            return false;
        }
    }
    base::LOG_DEBUG("post a new event : %d", context->_event_flag_set);
    return true;
}

bool CIOCP::_PostSend(base::CMemSharePtr<CEventHandler>& event) {
    EventOverlapped* context = (EventOverlapped*)event->_data;

    context->Clear();
    context->_event_flag_set = event->_event_flag_set;
    context->_wsa_buf.len = event->_buffer->Read(context->_lapped_buffer, __iocp_buff_size);
    
    OVERLAPPED *lapped = &context->_overlapped;
    auto socket_ptr = event->_client_socket.Lock();

    int res = WSASend(socket_ptr->GetSocket(), &context->_wsa_buf, 1, nullptr, 0, lapped, nullptr);

    if ((SOCKET_ERROR == res) && (WSA_IO_PENDING != WSAGetLastError())) {
        base::LOG_WARN("IOCP post send event failed! error code: %d", WSAGetLastError());
        return false;
    }
    base::LOG_DEBUG("post a new event : %d", context->_event_flag_set);
    return true;
}

bool CIOCP::_PostConnection(base::CMemSharePtr<CEventHandler>& event, const std::string& ip, short port, const char* buf, int buf_len) {
    EventOverlapped* context = (EventOverlapped*)event->_data;

    DWORD dwFlags = 0;
    DWORD dwBytes = 0;
    context->Clear();
    context->_event_flag_set = event->_event_flag_set;
    OVERLAPPED *lapped = &context->_overlapped;

    SOCKADDR_IN addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.S_un.S_addr = inet_addr(ip.c_str());

    SOCKADDR_IN local;
    local.sin_family = AF_INET;
    local.sin_port = htons(0);
    local.sin_addr.S_un.S_addr = INADDR_ANY;

    auto socket_ptr = event->_client_socket.Lock();
    if (SOCKET_ERROR == bind(socket_ptr->GetSocket(), (sockaddr*)&local, sizeof(local))) {
        base::LOG_FATAL("bind local host failed! error code: %d", WSAGetLastError());
    }

    int res = __ConnectEx(socket_ptr->GetSocket(), (sockaddr*)&addr, sizeof(addr), (PVOID)buf, buf_len, nullptr, lapped);

    if ((SOCKET_ERROR == res) && (WSA_IO_PENDING != WSAGetLastError())) {
        base::LOG_FATAL("IOCP post connect event failed! error code: %d", WSAGetLastError());
        return false;
    }
    base::LOG_DEBUG("post a new event : %d", context->_event_flag_set);
    return true;
}

bool CIOCP::_PostDisconnection(base::CMemSharePtr<CEventHandler>& event) {
    EventOverlapped* context = (EventOverlapped*)event->_data;

    context->Clear();
    context->_event_flag_set = event->_event_flag_set;

    OVERLAPPED *lapped = &context->_overlapped;
    auto socket_ptr = event->_client_socket.Lock();

    int res = __DisconnectionEx(socket_ptr->GetSocket(), lapped, 0, 0);

    if ((SOCKET_ERROR == res) && (WSA_IO_PENDING != WSAGetLastError())) {
        base::LOG_FATAL("IOCP post send event failed! error code: %d", WSAGetLastError());
        return false;
    }
    base::LOG_DEBUG("post a new event : %d", context->_event_flag_set);
    return true;
}

void CIOCP::_DoTimeoutEvent(std::vector<base::CMemSharePtr<CTimerEvent>>& timer_vec) {
    for (auto iter = timer_vec.begin(); iter != timer_vec.end(); ++iter) {
        if ((*iter)->_event_flag & EVENT_READ) {
            base::CMemSharePtr<CEventHandler> event_ptr = (*iter)->_event.Lock();
            base::CMemSharePtr<CSocketImpl> socket_ptr = event_ptr->_client_socket.Lock();
            if (socket_ptr) {
                event_ptr->_event_flag_set |= EVENT_TIMER;
                socket_ptr->Recv(event_ptr);
            }

        } else if ((*iter)->_event_flag & EVENT_WRITE) {
            base::CMemSharePtr<CEventHandler> event_ptr = (*iter)->_event.Lock();
            base::CMemSharePtr<CSocketImpl> socket_ptr = event_ptr->_client_socket.Lock();
            if (socket_ptr) {
                event_ptr->_event_flag_set |= EVENT_TIMER;
                socket_ptr->Send(event_ptr);
            }

        } else if ((*iter)->_event_flag & EVENT_TIMER) {
            auto func = (*iter)->_timer_call_back;
            if (func) {
                func((*iter)->_timer_param);
            }
        }
    }
    timer_vec.clear();
}

void CIOCP::_DoEvent(EventOverlapped *socket_context, int bytes) {
    if (socket_context->_event_flag_set & EVENT_ACCEPT) {
        base::CMemSharePtr<CAcceptEventHandler>* event = (base::CMemSharePtr<CAcceptEventHandler>*)socket_context->_event;
        if (event) {
            (*event)->_client_socket->_read_event->_off_set = bytes;
            (*event)->_accept_socket->_Accept((*event));
        }

    } else {
        base::CMemSharePtr<CEventHandler>* event = (base::CMemSharePtr<CEventHandler>*)socket_context->_event;
        if (event) {
            (*event)->_event_flag_set = socket_context->_event_flag_set;
            (*event)->_off_set = bytes;
            if (socket_context->_event_flag_set & EVENT_READ 
                || socket_context->_event_flag_set & EVENT_CONNECT 
                || socket_context->_event_flag_set & EVENT_DISCONNECT) {

                auto socket_ptr = (*event)->_client_socket.Lock();
                if (socket_ptr) {
                    socket_ptr->Recv((*event));
                }

            } else if ((*event)->_event_flag_set & EVENT_WRITE) {
                auto socket_ptr = (*event)->_client_socket.Lock();
                if (socket_ptr) {
                    socket_ptr->Send((*event));
                }
            }
        }
    }
}

void CIOCP::_DoTaskList() {
    std::vector<std::function<void(void)>> func_vec;
    {
        std::unique_lock<std::mutex> lock(_mutex);
        func_vec.swap(_task_list);
    }

    for (size_t i = 0; i < func_vec.size(); ++i) {
        func_vec[i]();
    }
}

bool CIOCP::_AddToActions(base::CMemSharePtr<CSocketImpl>& socket) {
    if (!socket->IsInActions()) {
        if (CreateIoCompletionPort((HANDLE)(socket->GetSocket()), _iocp_handler, 0, 0) == NULL) {
            base::LOG_ERROR("IOCP bind socket to io completion port failed!");
            return false;
        }
        socket->SetInActions(true);
    }
    return true;
}

#endif