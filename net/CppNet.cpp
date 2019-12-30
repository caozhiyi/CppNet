#include "CppNet.h"
#include "PoolSharedPtr.h"
#include "EventHandler.h"
#include "MemoryPool.h"
#include "Timer.h"
#include "CppNetImpl.h"
#include "Log.h"
#include "CNConfig.h"
#include "NetManager.h"

cppnet::Handle cppnet::Init(int32_t thread_num) {
    if (__open_log) {
        base::CLog::Instance().SetLogLevel(__log_level);
        base::CLog::Instance().SetLogName(__log_file_name);
        base::CLog::Instance().Start();
    }
    
    return cppnet::CCppNetManager::Instance().Init(thread_num);
}

void cppnet::Dealloc(Handle net_handle) {
    cppnet::CCppNetManager::Instance().Dealloc(net_handle);
    if (!base::CLog::Instance().GetStop()) {
        base::CLog::Instance().Stop();
    }
}

void cppnet::Join(Handle net_handle) {
    cppnet::CCppNetManager::Instance().Join(net_handle);
    if (!base::CLog::Instance().GetStop()) {
        base::CLog::Instance().Join();
    }
}

void cppnet::SetReadCallback(const read_call_back& func, Handle net_handle) {
    cppnet::CCppNetManager::Instance().SetReadCallback(func, net_handle);
}

void cppnet::SetWriteCallback(const write_call_back& func, Handle net_handle) {
    cppnet::CCppNetManager::Instance().SetWriteCallback(func, net_handle);
}

void cppnet::SetDisconnectionCallback(const connection_call_back& func, Handle net_handle) {
    cppnet::CCppNetManager::Instance().SetDisconnectionCallback(func, net_handle);
}

//timer
uint64_t cppnet::SetTimer(int32_t interval, const timer_call_back& func, void* param, bool always, Handle net_handle) {
    return cppnet::CCppNetManager::Instance().SetTimer(interval, func, param, always, net_handle);
}

void cppnet::RemoveTimer(uint64_t timer_id, Handle net_handle) {
    cppnet::CCppNetManager::Instance().RemoveTimer(timer_id, net_handle);
}

//server
void cppnet::SetAcceptCallback(const connection_call_back& func, Handle net_handle) {
    cppnet::CCppNetManager::Instance().SetAcceptCallback(func, net_handle);
}

bool cppnet::ListenAndAccept(const std::string& ip, int16_t port, Handle net_handle) {
    return cppnet::CCppNetManager::Instance().ListenAndAccept(ip, port, net_handle);
}

//client
void cppnet::SetConnectionCallback(const connection_call_back& func, Handle net_handle) {
    cppnet::CCppNetManager::Instance().SetConnectionCallback(func, net_handle);
}