#include "Socket.h"
#include "CppNetImpl.h"

int16_t cppnet::SyncRead(const Handle& handle) {
    auto socket = CCppNetImpl::Instance().GetSocket(handle);
    if (socket) {
        socket->SyncRead();
        return cppnet::EVENT_ERROR_NO;
    }
    return cppnet::EVENT_ERROR_INVALID_HANDLE;
}

int16_t cppnet::SyncWrite(const Handle& handle, char* src, int32_t len) {
    auto socket = CCppNetImpl::Instance().GetSocket(handle);
    if (socket) {
        socket->SyncWrite(src, len);
        return cppnet::EVENT_ERROR_NO;
    }
    return cppnet::EVENT_ERROR_INVALID_HANDLE;
}

int16_t cppnet::SyncRead(const Handle& handle, int32_t interval) {
    auto socket = CCppNetImpl::Instance().GetSocket(handle);
    if (socket) {
        socket->SyncRead(interval);
        return cppnet::EVENT_ERROR_NO;
    }
    return cppnet::EVENT_ERROR_INVALID_HANDLE;
}

int16_t cppnet::SyncWrite(const Handle& handle, int32_t interval, char* src, int32_t len) {
    auto socket = CCppNetImpl::Instance().GetSocket(handle);
    if (socket) {
        socket->SyncRead(interval);
        return cppnet::EVENT_ERROR_NO;
    }
    return cppnet::EVENT_ERROR_INVALID_HANDLE;
}

int16_t cppnet::PostTask(std::function<void(void)>& func) {
    return cppnet::EVENT_ERROR_INVALID_HANDLE;
}

#ifndef __linux__
// sync connection. 
int16_t cppnet::SyncConnection(const std::string& ip, int16_t port, char* buf, int32_t buf_len) {
    
}

#else
int16_t cppnet::SyncConnection(const std::string& ip, int16_t port)  {
    
}

#endif
int16_t cppnet::SyncDisconnection()  {
    
}


