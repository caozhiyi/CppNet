#include "Socket.h"
#include "CppNetImpl.h"

int16_t cppnet::GetIpAddress(const Handle& handle, std::string& ip, uint16_t& port) {
    auto socket = CCppNetImpl::Instance().GetSocket(handle);
        if (socket) {
            ip = socket->GetAddress();
            port = socket->GetPort();
            return cppnet::CEC_SUCCESS;
        }
        return cppnet::CEC_INVALID_HANDLE;
}

int16_t cppnet::Write(const Handle& handle, const char* src, int32_t len) {
    auto socket = CCppNetImpl::Instance().GetSocket(handle);
    if (socket) {
        socket->SyncWrite(src, len);
        return cppnet::CEC_SUCCESS;
    }
    return cppnet::CEC_INVALID_HANDLE;
}

int16_t cppnet::PostTask(std::function<void(void)>& func) {
    return cppnet::CEC_INVALID_HANDLE;
}

#ifndef __linux__
// sync connection. 
int16_t cppnet::Connection(const std::string& ip, int16_t port, const char* buf, int32_t buf_len) {
    if (CCppNetImpl::Instance().Connection(port, ip, buf, buf_len)) {
        return cppnet::CEC_SUCCESS;
    }
    return cppnet::CEC_FAILED;
}

#endif
int16_t cppnet::Connection(const std::string& ip, int16_t port) {
    if (CCppNetImpl::Instance().Connection(port, ip)) {
        return cppnet::CEC_SUCCESS;
    }
    return cppnet::CEC_FAILED;
}

int16_t cppnet::Close(const Handle& handle) {
    auto socket = CCppNetImpl::Instance().GetSocket(handle);
    if (socket) {
        socket->SyncDisconnection();
        return cppnet::CEC_SUCCESS;
    }
    return cppnet::CEC_INVALID_HANDLE;
}