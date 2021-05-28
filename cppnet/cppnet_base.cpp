// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#include "cppnet_base.h"
#include "cppnet/dispatcher.h"
#include "include/cppnet_type.h"
#include "cppnet/cppnet_config.h"
#include "cppnet/socket/rw_socket.h"

#include "common/log/log.h"
#include "common/os/os_info.h"
#include "common/util/random.h"
#include "common/network/socket.h"
#include "common/network/io_handle.h"
#include "common/buffer/buffer_queue.h"


namespace cppnet {

union TimerId {
    struct {
        uint32_t _timer_id;
        uint32_t _dispatcher_index;
    } _detail_info;
    uint64_t _timer_id;
};


CppNetBase::CppNetBase() {

}

CppNetBase::~CppNetBase() {

}

void CppNetBase::Init(uint32_t thread_num) {
    uint32_t cpus = GetCpuNum();
    if (thread_num == 0 || thread_num > cpus * 2) {
        thread_num = cpus;
    }
    _random = std::make_shared<RangeRandom>(0, thread_num - 1);

#ifdef __win__
    static uint32_t __cppnet_base_id = 0;
    __cppnet_base_id++;
    for (uint32_t i = 0; i < thread_num; i++) {
        auto dispatcher = std::make_shared<Dispatcher>(shared_from_this(), thread_num, __cppnet_base_id);
        _dispatchers.push_back(dispatcher);
    }

#else
    for (uint32_t i = 0; i < thread_num; i++) {
        auto dispatcher = std::make_shared<Dispatcher>(shared_from_this());
        _dispatchers.push_back(dispatcher);
    }

#endif

}

void CppNetBase::Dealloc() {
    for (size_t i = 0; i < _dispatchers.size(); i++) {
        _dispatchers[i]->Stop();
    }
}

void CppNetBase::Join() {
    for (size_t i = 0; i < _dispatchers.size(); i++) {
        _dispatchers[i]->Join();
    }
}

uint64_t CppNetBase::AddTimer(uint32_t interval, const user_timer_call_back& cb, void* param, bool always) {
    uint32_t index = _random->Random();
    uint32_t id = _dispatchers[index]->AddTimer(cb, param, interval, always);
    TimerId tid;
    tid._detail_info._dispatcher_index = index;
    tid._detail_info._timer_id = id;
    return tid._timer_id;
}

void CppNetBase::RemoveTimer(uint64_t timer_id) {
    TimerId tid;
    tid._timer_id = timer_id;
    _dispatchers[tid._detail_info._dispatcher_index]->StopTimer(tid._detail_info._timer_id);
}

bool CppNetBase::ListenAndAccept(const std::string& ip, uint16_t port) {
    if (__reuse_port) {
        for (size_t i = 0; i < _dispatchers.size(); i++) {
            auto ret = OsHandle::TcpSocket();
            if (ret._return_value < 0) {
                LOG_ERROR("create socket failed. err:%d", ret._errno);
                return false;
            }
            ReusePort(ret._return_value);
            _dispatchers[i]->Listen(ret._return_value, ip, port);
        }

    } else {
        auto ret = OsHandle::TcpSocket();
        if (ret._return_value < 0) {
            LOG_ERROR("create socket failed. err:%d", ret._errno);
            return false;
        }
        for (size_t i = 0; i < _dispatchers.size(); i++) {
            _dispatchers[i]->Listen(ret._return_value, ip, port);
        }
    }
    return true;
}

bool CppNetBase::Connection(const std::string& ip, uint16_t port) {
    uint32_t index = _random->Random();
    _dispatchers[index]->Connect(ip, port);
    return true;
}

void CppNetBase::OnTimer(std::shared_ptr<RWSocket> sock) {
    if (_timer_cb) {
        _timer_cb(sock);
    }
}

void CppNetBase::OnAccept(std::shared_ptr<RWSocket> sock) {
    if (_accept_cb) {
        _accept_cb(sock, CEC_SUCCESS);
    }
}

void CppNetBase::OnRead(std::shared_ptr<RWSocket> sock, std::shared_ptr<Buffer> buffer, uint32_t len) {
    if (_read_cb) {
        _read_cb(sock, buffer, len);
    }
}

void CppNetBase::OnWrite(std::shared_ptr<RWSocket> sock, uint32_t len) {
    if (_write_cb) {
        _write_cb(sock, len);
    }
}

void CppNetBase::OnConnect(std::shared_ptr<RWSocket> sock, uint16_t err) {
    if (_connect_cb) {
        _connect_cb(sock, err);
    }
}

void CppNetBase::OnDisConnect(std::shared_ptr<RWSocket> sock, uint16_t err) {
    if (_disconnect_cb) {
        _disconnect_cb(sock, err);
    }
}

}