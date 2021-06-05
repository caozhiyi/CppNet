// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#include "include/cppnet.h"

#include "cppnet_base.h"
#include "cppnet/cppnet_config.h"

#include "common/log/log.h"
#include "common/log/file_logger.h"
#include "common/log/stdout_logger.h"

namespace cppnet {

CppNet::CppNet() {

}

CppNet::~CppNet() {
    if (_cppnet_base) {
        _cppnet_base->Dealloc();
    }
}

void CppNet::Init(int32_t thread_num) {
    if (!_cppnet_base) {
        _cppnet_base = std::make_shared<CppNetBase>();

    } else {
        return;
    }
    
    _cppnet_base->Init(thread_num);
    if (__print_log) {
        std::shared_ptr<Logger> file_log = std::make_shared<FileLogger>(__log_file_name);
        std::shared_ptr<Logger> std_log = std::make_shared<StdoutLogger>();
        file_log->SetLogger(std_log);
        LOG_SET(file_log);
        LOG_SET_LEVEL((LogLevel)__log_level);
    } else {
        LOG_SET_LEVEL(LL_NULL);
    }
}

void CppNet::Destory() {
    _cppnet_base->Dealloc();
    _cppnet_base.reset();
}

void CppNet::Join() {
    _cppnet_base->Join();
}

void CppNet::SetReadCallback(const read_call_back& cb) {
    _cppnet_base->SetReadCallback(cb);
}

void CppNet::SetWriteCallback(const write_call_back& cb) {
    _cppnet_base->SetWriteCallback(cb);
}

void CppNet::SetDisconnectionCallback(const connect_call_back& cb) {
    _cppnet_base->SetDisconnectionCallback(cb);
}

void CppNet::SetTimerCallback(const timer_call_back& cb) {
    _cppnet_base->SetTimerCallback(cb);
}

uint64_t CppNet::AddTimer(int32_t interval, const user_timer_call_back& cb, void* param, bool always) {
    return _cppnet_base->AddTimer(interval, cb, param, always);
}

void CppNet::RemoveTimer(uint64_t timer_id) {
    _cppnet_base->RemoveTimer(timer_id);
}

void CppNet::SetAcceptCallback(const connect_call_back& cb) {
    _cppnet_base->SetAcceptCallback(cb);
}

bool CppNet::ListenAndAccept(const std::string& ip, uint16_t port) {
    return _cppnet_base->ListenAndAccept(ip, port);
}

void CppNet::SetConnectionCallback(const connect_call_back& cb) {
    _cppnet_base->SetConnectionCallback(cb);
}

bool CppNet::Connection(const std::string& ip, uint16_t port) {
    return _cppnet_base->Connection(ip, port);
}

}