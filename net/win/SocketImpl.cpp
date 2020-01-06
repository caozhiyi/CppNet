#ifndef __linux__
#include "Log.h"
#include "IOCP.h"
#include "Buffer.h"
#include "SocketImpl.h"
#include "CppNetImpl.h"
#include "EventActions.h"
#include "EventHandler.h"
#include "WinExpendFunc.h"
#include "CallBackHandle.h"

using namespace cppnet;

CSocketImpl::CSocketImpl(std::shared_ptr<CEventActions>& event_actions, uint32_t net_index, std::shared_ptr<CallBackHandle>& call_back_handle) : 
                CSocketBase(event_actions, net_index, call_back_handle), 
                _post_event_num(0) {
    _read_event = base::MakeNewSharedPtr<CEventHandler>(_pool.get());
    _write_event = base::MakeNewSharedPtr<CEventHandler>(_pool.get());

    _read_event->_data = _pool->PoolNew<EventOverlapped>();
    _read_event->_buffer = base::MakeNewSharedPtr<base::CBuffer>(_pool.get(), _pool);

    _write_event->_data = _pool->PoolNew<EventOverlapped>();
    _write_event->_buffer = base::MakeNewSharedPtr<base::CBuffer>(_pool.get(), _pool);
}

CSocketImpl::~CSocketImpl() {
    // remove from iocp
    base::LOG_DEBUG("close a socket, socket : %d, TheadId : %lld", _sock, std::this_thread::get_id());
    if (_read_event && _read_event->_data) {
        EventOverlapped* temp = (EventOverlapped*)_read_event->_data;
        _pool->PoolDelete<EventOverlapped>(temp);
        _read_event->_data = nullptr;
    }
    if (_write_event && _write_event->_data) {
        EventOverlapped* temp = (EventOverlapped*)_write_event->_data;
        _pool->PoolDelete<EventOverlapped>(temp);
        _write_event->_data = nullptr;
    }
}

void CSocketImpl::SyncRead() {
    if (!_read_event->_client_socket) {
        _read_event->_client_socket = memshared_from_this();
    }

    if (_event_actions) {
        _read_event->_event_flag_set = 0;
        _read_event->_event_flag_set |= EVENT_READ;
        if (_event_actions->AddRecvEvent(_read_event)) {
            _post_event_num++;

        // something wrong
        }else {
            _callback_handle->_read_call_back(_read_event, ERR_CONNECT_CLOSE | EVENT_DISCONNECT);
        }
    }
}

void CSocketImpl::SyncWrite(const char* src, uint32_t len) {
    _write_event->_buffer->Write(src, len);

    if (!_write_event->_client_socket) {
        _write_event->_client_socket = memshared_from_this();
    }

    if (_event_actions) {
        _write_event->_event_flag_set = 0;
        _write_event->_event_flag_set |= EVENT_WRITE;
        if (_event_actions->AddSendEvent(_write_event)) {
            _post_event_num++;

        // something wrong
        } else {
            _callback_handle->_read_call_back(_read_event, ERR_CONNECT_CLOSE | EVENT_DISCONNECT);
        }
    }
}

void CSocketImpl::SyncConnection(const std::string& ip, uint16_t port, const char* buf, uint32_t buf_len) {
    if (ip.length() > 16 || ip.empty()) {
        base::LOG_ERROR("a wrong ip! ip : %s", ip.c_str());
        return;
    }

    if (!_read_event->_client_socket) {
        _read_event->_client_socket = memshared_from_this();
    }

    // set address info
    strcpy(_ip, ip.c_str());
    _port = port;

    if (_event_actions) {
        _read_event->_event_flag_set = 0;
        _read_event->_event_flag_set |= EVENT_CONNECT;
        if (_event_actions->AddConnection(_read_event, ip, port, buf, buf_len)) {
            _post_event_num++;
        }
    }
}

void CSocketImpl::SyncConnection(const std::string& ip, uint16_t port) {
    SyncConnection(ip, port, nullptr, 0);
}

void CSocketImpl::SyncDisconnection() {
    if (!_read_event->_client_socket) {
        _read_event->_client_socket = memshared_from_this();
    }

    if (_event_actions) {
        _read_event->_event_flag_set = 0;
        _read_event->_event_flag_set |= EVENT_DISCONNECT;
        if (_event_actions->AddDisconnection(_read_event)) {
            _post_event_num++;

        } else {
            _callback_handle->_write_call_back(_write_event, _write_event->_event_flag_set | ERR_CONNECT_CLOSE | EVENT_DISCONNECT);
        }
    }
}

void CSocketImpl::PostTask(std::function<void(void)>& func) {
    _event_actions->PostTask(func);
}

void CSocketImpl::Recv(base::CMemSharePtr<CEventHandler>& event) {
    EventOverlapped* context = (EventOverlapped*)event->_data;
    _post_event_num--;
    int err = event->_event_flag_set;
    if (event->_event_flag_set & EVENT_TIMER) {
        err |= ERR_TIME_OUT;

    //get a connection event
    } else if (event->_event_flag_set & EVENT_CONNECT) {
        // do nothing

    } else if (event->_event_flag_set & EVENT_DISCONNECT) {
        if (_post_event_num == 0) {
            err |= ERR_CONNECT_CLOSE;
        }

    //get 0 bytes means close
    } else if (!event->_off_set) {
        // close when all event return.
        if (_post_event_num == 0) {
            err = EVENT_DISCONNECT | ERR_CONNECT_CLOSE;
            _callback_handle->_read_call_back(event, err);
            event->_event_flag_set = 0;
        }
        return;

    } else {
        event->_buffer->Write(context->_wsa_buf.buf, event->_off_set);
    }
    if (err > -1) {
        _callback_handle->_read_call_back(event, err);
        event->_event_flag_set = 0;
    }
}

void CSocketImpl::Send(base::CMemSharePtr<CEventHandler>& event) {
    EventOverlapped* context = (EventOverlapped*)event->_data;

    _post_event_num--;
    int err = event->_event_flag_set;
    if (event->_event_flag_set & EVENT_TIMER) {
        err |= ERR_TIME_OUT; 

    } else if (!event->_off_set) {
        if (_post_event_num == 0) {
            err |= ERR_CONNECT_CLOSE;
        }
    }

    if (err > -1) {
        _callback_handle->_write_call_back(event, err);
        event->_event_flag_set = 0;
    }
}

#endif