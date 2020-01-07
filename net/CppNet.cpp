#include "Log.h"
#include "Timer.h"
#include "CppNet.h"
#include "CNConfig.h"
#include "CppNetImpl.h"
#include "MemoryPool.h"
#include "EventHandler.h"
#include "PoolSharedPtr.h"

void cppnet::Init(int32_t thread_num) {
    if (__open_log) {
        base::CLog::Instance().SetLogLevel(__log_level);
        base::CLog::Instance().SetLogName(__log_file_name);
        base::CLog::Instance().Start();
    }
    
    cppnet::CCppNetImpl::Instance().Init(thread_num);
}

void cppnet::Dealloc() {
    cppnet::CCppNetImpl::Instance().Dealloc();
    if (!base::CLog::Instance().GetStop()) {
        base::CLog::Instance().Stop();
    }
}

void cppnet::Join() {
    cppnet::CCppNetImpl::Instance().Join();
    if (!base::CLog::Instance().GetStop()) {
        base::CLog::Instance().Join();
    }
}

void cppnet::SetReadCallback(const read_call_back& func) {
    cppnet::CCppNetImpl::Instance().SetReadCallback(func);
}

void cppnet::SetWriteCallback(const write_call_back& func) {
    cppnet::CCppNetImpl::Instance().SetWriteCallback(func);
}

void cppnet::SetDisconnectionCallback(const connection_call_back& func) {
    cppnet::CCppNetImpl::Instance().SetDisconnectionCallback(func);
}

//timer
uint64_t cppnet::SetTimer(int32_t interval, const timer_call_back& func, void* param, bool always) {
    return cppnet::CCppNetImpl::Instance().SetTimer(interval, func, param, always);
}

void cppnet::RemoveTimer(uint64_t timer_id) {
    cppnet::CCppNetImpl::Instance().RemoveTimer(timer_id);
}

//server
void cppnet::SetAcceptCallback(const connection_call_back& func) {
    cppnet::CCppNetImpl::Instance().SetAcceptCallback(func);
}

bool cppnet::ListenAndAccept(const std::string& ip, int16_t port) {
    return cppnet::CCppNetImpl::Instance().ListenAndAccept(ip, port);
}

//client
void cppnet::SetConnectionCallback(const connection_call_back& func) {
    cppnet::CCppNetImpl::Instance().SetConnectionCallback(func);
}