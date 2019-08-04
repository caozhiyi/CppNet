#include "CppNet.h"
#include "PoolSharedPtr.h"
#include "EventHandler.h"
#include "MemaryPool.h"
#include "Timer.h"
#include "CppNetImpl.h"
#include "Log.h"

void cppnet::Init(int thread_num, bool log) {
    if (log) {
        base::CLog::Instance().SetLogLevel(base::LOG_WARN_LEVEL);
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

void cppnet::SetReadCallback(const call_back& func) {
    cppnet::CCppNetImpl::Instance().SetReadCallback(func);
}

void cppnet::SetWriteCallback(const call_back& func) {
    cppnet::CCppNetImpl::Instance().SetWriteCallback(func);
}

void cppnet::SetDisconnectionCallback(const call_back& func) {
    cppnet::CCppNetImpl::Instance().SetDisconnectionCallback(func);
}

//timer
uint64_t cppnet::SetTimer(unsigned int interval, const std::function<void(void*)>& func, void* param, bool always) {
    return cppnet::CCppNetImpl::Instance().SetTimer(interval, func, param, always);
}

void cppnet::RemoveTimer(uint64_t timer_id) {
    cppnet::CCppNetImpl::Instance().RemoveTimer(timer_id);
}

//server
void cppnet::SetAcceptCallback(const call_back& func) {
    cppnet::CCppNetImpl::Instance().SetAcceptCallback(func);
}

bool cppnet::ListenAndAccept(int port, std::string ip) {
    return cppnet::CCppNetImpl::Instance().ListenAndAccept(port, ip);
}

//client
void cppnet::SetConnectionCallback(const call_back& func) {
    cppnet::CCppNetImpl::Instance().SetConnectionCallback(func);
}

base::CMemSharePtr<cppnet::CSocket> cppnet::Connection(int port, std::string ip, char* buf, int buf_len) {
    return cppnet::CCppNetImpl::Instance().Connection(port, ip, buf, buf_len);
}

base::CMemSharePtr<cppnet::CSocket> cppnet::Connection(int port, std::string ip) {
    return cppnet::CCppNetImpl::Instance().Connection(port, ip);
}