#include "CppNet.h"
#include "PoolSharedPtr.h"
#include "EventHandler.h"
#include "MemaryPool.h"
#include "Timer.h"
#include "CppNetImpl.h"
#include "Log.h"

void cppnet::Init(int32_t thread_num, bool log, bool per_handl_thread) {
    if (log) {
        base::CLog::Instance().SetLogLevel(base::LOG_DEBUG_LEVEL);
        base::CLog::Instance().SetLogName("CppNet.txt");
        base::CLog::Instance().Start();
    }
    
    cppnet::CCppNetImpl::Instance().Init(thread_num);
}

void cppnet::Dealloc() {
    cppnet::CCppNetImpl::Instance().Dealloc();
}

void cppnet::Join() {
    if (!base::CLog::Instance().GetStop()) {
        base::CLog::Instance().Join();
    }
    cppnet::CCppNetImpl::Instance().Join();
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

bool cppnet::ListenAndAccept(int16_t port, std::string ip, uint32_t listen_num) {
    return cppnet::CCppNetImpl::Instance().ListenAndAccept(port, ip, listen_num);
}

//client
void cppnet::SetConnectionCallback(const connection_call_back& func) {
    cppnet::CCppNetImpl::Instance().SetConnectionCallback(func);
}