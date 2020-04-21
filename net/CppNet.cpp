#include "Log.h"
#include "Timer.h"
#include "CppNet.h"
#include "CNConfig.h"
#include "CppNetImpl.h"
#include "MemoryPool.h"
#include "EventHandler.h"
#include "PoolSharedPtr.h"

cppnet::CCppNet::CCppNet() {
    _cppnet_instance.reset(new CCppNetImpl());
}

cppnet::CCppNet::~CCppNet() {
    _cppnet_instance->Dealloc();
    if (!base::CLog::Instance().GetStop()) {
        base::CLog::Instance().Stop();
    }
}

void cppnet::CCppNet::Init(int32_t thread_num) {
    if (__open_log) {
        base::CLog::Instance().SetLogLevel(__log_level);
        base::CLog::Instance().SetLogName(__log_file_name);
        base::CLog::Instance().Start();
    }
    
    _cppnet_instance->Init(thread_num);
}

void cppnet::CCppNet::Join() {
    _cppnet_instance->Join();
    if (!base::CLog::Instance().GetStop()) {
        base::CLog::Instance().Join();
    }
}

void cppnet::CCppNet::SetReadCallback(const read_call_back& func) {
    _cppnet_instance->SetReadCallback(func);
}

void cppnet::CCppNet::SetWriteCallback(const write_call_back& func) {
    _cppnet_instance->SetWriteCallback(func);
}

void cppnet::CCppNet::SetDisconnectionCallback(const connection_call_back& func) {
    _cppnet_instance->SetDisconnectionCallback(func);
}

uint64_t cppnet::CCppNet::SetTimer(int32_t interval, const timer_call_back& func, void* param, bool always) {
    return _cppnet_instance->SetTimer(interval, func, param, always);
}

void cppnet::CCppNet::RemoveTimer(uint64_t timer_id) {
    _cppnet_instance->RemoveTimer(timer_id);
}

void cppnet::CCppNet::SetAcceptCallback(const connection_call_back& func) {
    _cppnet_instance->SetAcceptCallback(func);
}

bool cppnet::CCppNet::ListenAndAccept(const std::string& ip, int16_t port) {
    return _cppnet_instance->ListenAndAccept(ip, port);
}

void cppnet::CCppNet::SetConnectionCallback(const connection_call_back& func) {
    _cppnet_instance->SetConnectionCallback(func);
}

#ifndef __linux__
bool cppnet::CCppNet::Connection(const std::string& ip, int16_t port, const char* buf, int32_t buf_len) {
    return _cppnet_instance->Connection(ip, port, buf, buf_len);
}
#endif
bool cppnet::CCppNet::Connection(const std::string& ip, int16_t port) {
    return _cppnet_instance->Connection(ip, port);
}