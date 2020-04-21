#include <random>
#ifdef __linux__
#include <sys/socket.h>
#endif

#include "Log.h"
#include "OSInfo.h"
#include "CNConfig.h"
#include "Runnable.h"
#include "SocketImpl.h"
#include "CppNetImpl.h"
#include "EventActions.h"

#ifdef __linux__
#include "CEpoll.h"
#include "LinuxFunc.h"
#else
#include "IOCP.h"
#endif

using namespace cppnet;

CCppNetImpl::CCppNetImpl() : _pool(__mem_block_size, __mem_block_add_step) {

}

CCppNetImpl::~CCppNetImpl() {
    Join();
    _thread_vec.clear();
    _actions_map.clear();
    _accept_socket.clear();
}

void CCppNetImpl::Init(uint32_t thread_num) {
#ifndef __linux__
    InitScoket();
#else
    SetCoreFileUnlimit();
#endif // __linux__

    uint32_t cpus = GetCpuNum();
    if (thread_num == 0 || thread_num > cpus * 2) {
        thread_num = cpus;
    }

#ifdef __linux__
    if (__per_handle_thread) {
        for (size_t i = 0; i < thread_num; i++) {
            // create a epoll
            std::shared_ptr<CEventActions> event_actions(new CEpoll(__per_handle_thread));
            // start net io thread
            event_actions->Init(thread_num);
            std::shared_ptr<std::thread> thd(new std::thread(std::bind(&CEventActions::ProcessEvent, event_actions)));
            _actions_map[thd->get_id()] = event_actions;
            _thread_vec.push_back(thd);
        }
    } else {
        // create only one epoll
        std::shared_ptr<CEventActions> event_actions(new CEpoll(__per_handle_thread));
        // start net io thread
        event_actions->Init(thread_num);
        
        for (size_t i = 0; i < thread_num; i++) {
            std::shared_ptr<std::thread> thd(new std::thread(std::bind(&CEventActions::ProcessEvent, event_actions)));
            _actions_map[thd->get_id()] = event_actions;
            _thread_vec.push_back(thd);
        }
    }
#else
    // only one iocp
    std::shared_ptr<CEventActions> event_actions(new CIOCP);
    // start net io thread
    event_actions->Init(thread_num);
    for (size_t i = 0; i < thread_num; i++) {
        std::shared_ptr<std::thread> thd(new std::thread(std::bind(&CEventActions::ProcessEvent, event_actions)));
        _actions_map[thd->get_id()] = event_actions;
        _thread_vec.emplace_back(thd);
    }
#endif
}

void CCppNetImpl::Dealloc() {
    for (auto iter = _actions_map.begin(); iter != _actions_map.end(); ++iter) {
        iter->second->Dealloc();
    }
#ifndef __linux__
    DeallocSocket();
#endif // __linux__
}

void CCppNetImpl::Join() {
    for (size_t i = 0; i < _thread_vec.size(); ++i) {
        if (_thread_vec[i] && _thread_vec[i]->joinable()) {
            _thread_vec[i]->join();
        }
    }
    _thread_vec.clear();
    _actions_map.clear();
}

void CCppNetImpl::SetReadCallback(const read_call_back& func) {
    _read_call_back = func;
}

void CCppNetImpl::SetWriteCallback(const write_call_back& func) {
    _write_call_back = func;
}

void CCppNetImpl::SetDisconnectionCallback(const connection_call_back& func) {
    _disconnection_call_back = func;
}

uint64_t CCppNetImpl::SetTimer(uint32_t interval, const std::function<void(void*)>& func, void* param, bool always) {

    auto actions = _RandomGetActions();

    uint64_t timer_id = 0;
    timer_id = actions->AddTimerEvent(interval, func, param, always);
    actions->WakeUp();

    _timer_actions_map[timer_id] = actions;

    return timer_id;
}

void CCppNetImpl::RemoveTimer(uint64_t timer_id) {
    auto iter = _timer_actions_map.find(timer_id);
    if (iter != _timer_actions_map.end()) {
        auto actions = iter->second.lock();
        if (actions) {
            actions->RemoveTimerEvent(timer_id);
        }
    }
}

void CCppNetImpl::SetAcceptCallback(const connection_call_back& func) {
    _accept_call_back = func;
}

bool CCppNetImpl::ListenAndAccept(const std::string& ip, uint16_t port) {
    if (!_accept_call_back) {
        base::LOG_ERROR("accept call back function is null!, port : %d, ip : %s ", port, ip.c_str());
        return false;
    }

    if (!_read_call_back) {
        base::LOG_ERROR("read call back function is null!, port : %d, ip : %s ", port, ip.c_str());
        return false;
    }

    if (_actions_map.size() <= 0) {
        base::LOG_ERROR("CCppNetImpl obj is not init!, port : %d, ip : %s ", port, ip.c_str());
        return false;
    }

    for (auto iter = _actions_map.begin(); iter != _actions_map.end(); ++iter) {
        base::CMemSharePtr<CAcceptSocket> accept_socket = base::MakeNewSharedPtr<CAcceptSocket>(&_pool, iter->second);
        accept_socket->SetCppnetInstance(shared_from_this());
        if (!accept_socket->Bind(port, ip)) {
            base::LOG_ERROR("bind failed. port : %d, ip : %s ", port, ip.c_str());
            return false;
        }

        if (!accept_socket->Listen()) {
            base::LOG_ERROR("listen failed. port : %d, ip : %s ", port, ip.c_str());
            return false;
        }

        accept_socket->SyncAccept();
        _accept_socket[accept_socket->GetSocket()] = accept_socket;
#ifndef __linux__
        // only iocp handle on windows.
        break;
#else
        if (!__per_handle_thread) {
            break;
        }
#endif
    }
    return true;
}

void CCppNetImpl::SetConnectionCallback(const connection_call_back& func) {
    _connection_call_back = func;
}

#ifndef __linux__
bool CCppNetImpl::Connection(const std::string& ip, uint16_t port, const char* buf, uint32_t buf_len) {
    if (!_connection_call_back) {
        base::LOG_ERROR("connection call back function is null!, port : %d, ip : %s ", port, ip.c_str());
        return false;
    }
    if (!_read_call_back) {
        base::LOG_ERROR("read call back function is null!, port : %d, ip : %s ", port, ip.c_str());
        return false;
    }

    auto actions = _RandomGetActions();
    base::CMemSharePtr<CSocketImpl> sock = base::MakeNewSharedPtr<CSocketImpl>(&_pool, actions);
    sock->SetCppnetInstance(shared_from_this());
    sock->SyncConnection(ip, port, buf, buf_len);

    std::unique_lock<std::mutex> lock(_mutex);
    _socket_map[sock->GetSocket()] = sock;
    return true;
}
#endif

bool CCppNetImpl::Connection(const std::string& ip, uint16_t port) {
    if (!_connection_call_back) {
        base::LOG_ERROR("connection call back function is null!, port : %d, ip : %s ", port, ip.c_str());
        return 0;
    }
    if (!_read_call_back) {
        base::LOG_ERROR("read call back function is null!, port : %d, ip : %s ", port, ip.c_str());
        return 0;
    }

    auto actions = _RandomGetActions();
    base::CMemSharePtr<CSocketImpl> sock = base::MakeNewSharedPtr<CSocketImpl>(&_pool, actions);
    sock->SetCppnetInstance(shared_from_this());
#ifndef __linux__
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _socket_map[sock->GetSocket()] = sock;
    }
    sock->SyncConnection(ip, port, "", 0);
#else
    //create socket
    auto temp_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (temp_socket < 0) {
        base::LOG_ERROR("create socket failed. errno : %d ", errno);
        return 0;
    }
    sock->SetSocket(temp_socket);
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _socket_map[sock->GetSocket()] = sock;
    }
    sock->SyncConnection(ip, port);
#endif
    return sock->GetSocket();
}

base::CMemSharePtr<CSocketImpl> CCppNetImpl::GetSocket(uint64_t handle) {
    std::unique_lock<std::mutex> lock(_mutex);
    auto iter = _socket_map.find(handle);
    if (iter != _socket_map.end()) {
        return iter->second;
    }
    return nullptr;
}

bool CCppNetImpl::RemoveSocket(uint64_t handle) {
    std::unique_lock<std::mutex> lock(_mutex);
    auto iter = _socket_map.find(handle);
    if (iter != _socket_map.end()) {
        _socket_map.erase(iter);
        return true;
    }
    return false;
}

uint32_t CCppNetImpl::GetThreadNum() {
    return (uint32_t)_thread_vec.size();
}

void CCppNetImpl::_AcceptFunction(base::CMemSharePtr<CSocketImpl>& sock, uint32_t err) {
    if (!sock) {
        base::LOG_WARN("event is null while accept.");
        return;
    }
    
    uint64_t handle = sock->GetSocket();
    // create upper socket
    std::shared_ptr<CNSocket> cn_sock(new CNSocket());
    cn_sock->_cppnet_instance = shared_from_this();
    cn_sock->_socket_handle   = handle;
    sock->_upper_sock = cn_sock;
    {
        // add socket to map
        std::unique_lock<std::mutex> lock(_mutex);
        _socket_map[handle] = sock;
        base::LOG_DEBUG("get client num : %d", int(_socket_map.size()));
    }
    err = CEC_SUCCESS;
    if (_accept_call_back) {
        _accept_call_back(cn_sock, err);
    }
}

void CCppNetImpl::_ReadFunction(base::CMemSharePtr<CEventHandler>& event, uint32_t err) {
    if (!event) {
        base::LOG_WARN("event is null while read.");
        return;
    }
    auto socket_ptr = event->_client_socket.Lock();
    uint64_t handle = socket_ptr->GetSocket();
    if (err & EVENT_CONNECT) {
        // remote refuse connect
        if (err & ERR_CONNECT_FAILED || err & ERR_CONNECT_CLOSE) {
            err = CEC_CONNECT_REFUSE;

        } else {
            err = CEC_SUCCESS;

        }
        if (_connection_call_back) {
            _connection_call_back(socket_ptr->_upper_sock, err);
        }

        // start read
        if (err == CEC_SUCCESS) {
            socket_ptr->SyncRead();
            
        } else {
            std::unique_lock<std::mutex> lock(_mutex);
            _socket_map.erase(socket_ptr->GetSocket());
        }

    } else if (err & EVENT_DISCONNECT) {
        if (err & ERR_CONNECT_CLOSE) {
            err = CEC_SUCCESS;
            if (_disconnection_call_back) {
                _disconnection_call_back(socket_ptr->_upper_sock, err);
            }
            std::unique_lock<std::mutex> lock(_mutex);
            _socket_map.erase(socket_ptr->GetSocket());
        }

    } else if (err & EVENT_READ) {
        if (err & ERR_CONNECT_CLOSE) {
            err = CEC_CLOSED;

        } else if (err & ERR_CONNECT_BREAK) {
            err = CEC_CONNECT_BREAK;
            
        } else if (err & ERR_TIME_OUT) {
            err = CEC_TIMEOUT;

        } else {
            err = CEC_SUCCESS;
        }

        // call read back function 
        if (_read_call_back) {
            _read_call_back(socket_ptr->_upper_sock, socket_ptr->_read_event->_buffer.Get(), socket_ptr->_read_event->_off_set, err);
        }
        
#ifndef __linux__
        // post read event every time on windows.
        if (err != CEC_CLOSED && err != CEC_CONNECT_BREAK) {
            socket_ptr->SyncRead();

        } else if (err == CEC_CLOSED || err == CEC_CONNECT_BREAK) {
            socket_ptr->SyncDisconnection();
            return;
        }
    }
#else
        if (__per_handle_thread && err != CEC_CLOSED && err != CEC_CONNECT_BREAK) {
            socket_ptr->SyncRead();
        }  
    }
    if (err == CEC_CLOSED 
        || err == CEC_CONNECT_BREAK 
        || err == CEC_CONNECT_REFUSE) {
        std::unique_lock<std::mutex> lock(_mutex);
        _socket_map.erase(socket_ptr->GetSocket());
    }
#endif
}

void CCppNetImpl::_WriteFunction(base::CMemSharePtr<CEventHandler>& event, uint32_t err) {
    if (!event) {
        base::LOG_WARN("event is null while write.");
        return;
    }

    auto socket_ptr = event->_client_socket.Lock();
    uint64_t handle = socket_ptr->GetSocket();
    if (err & EVENT_WRITE) {
        if (err & ERR_CONNECT_CLOSE) {
            err = CEC_CLOSED;

        } else if (err & ERR_CONNECT_BREAK) {
            err = CEC_CONNECT_BREAK;
            
        } else if (err & ERR_TIME_OUT) {
            err = CEC_TIMEOUT;

        } else {
            err = CEC_SUCCESS;
        }
        if (_write_call_back) {
            _write_call_back(socket_ptr->_upper_sock, socket_ptr->_read_event->_off_set, err);
        }
#ifndef __linux__
        if (err == CEC_CLOSED || err == CEC_CONNECT_BREAK) {
            socket_ptr->SyncDisconnection();
            return;
        } else {
            if (socket_ptr->GetPoolSize() >= __max_block_num) {
                socket_ptr->ReleasePoolHalf();
            }
        }
    }
#else
    }

    if (err == CEC_CLOSED
       || err == CEC_CONNECT_BREAK) {
        std::unique_lock<std::mutex> lock(_mutex);
        _socket_map.erase(socket_ptr->GetSocket());

    } else {
        if (socket_ptr->GetPoolSize() >= __max_block_num) {
            socket_ptr->ReleasePoolHalf();
        }
    }
#endif
}

std::shared_ptr<CEventActions>& CCppNetImpl::_RandomGetActions() {
    static std::random_device rd;
    static std::mt19937 mt(rd());
    int index = mt() % int(_actions_map.size());
    auto iter = _actions_map.begin();
    for (int i = 0; i < index; i++) {
        iter++;
    }
    return iter->second;
}
