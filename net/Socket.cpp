#include "Socket.h"
#include "CppNetImpl.h"

cppnet::CNSocket::CNSocket() {

}

cppnet::CNSocket::~CNSocket() {

}

int16_t cppnet::CNSocket::GetAddress(std::string& ip, uint16_t& port) {
    auto cppnet_ins = _cppnet_instance.lock();
    if (!cppnet_ins) {
        return cppnet::CEC_FAILED;
    }
    auto socket = cppnet_ins->GetSocket(_socket_handle);
    if (socket) {
        ip = socket->GetAddress();
        port = socket->GetPort();
        return cppnet::CEC_SUCCESS;
    }
    return cppnet::CEC_FAILED;
}

int16_t cppnet::CNSocket::Write(const char* src, int32_t len) {
    auto cppnet_ins = _cppnet_instance.lock();
    if (!cppnet_ins) {
        return cppnet::CEC_FAILED;
    }
    auto socket = cppnet_ins->GetSocket(_socket_handle);
    if (socket) {
        socket->SyncWrite(src, len);
        return cppnet::CEC_SUCCESS;
    }
    return cppnet::CEC_FAILED;
}

int16_t cppnet::CNSocket::Close() {
    auto cppnet_ins = _cppnet_instance.lock();
    if (!cppnet_ins) {
        return cppnet::CEC_FAILED;
    }
    auto socket = cppnet_ins->GetSocket(_socket_handle);
    if (socket) {
        socket->SyncDisconnection();
        return cppnet::CEC_SUCCESS;
    }
    return cppnet::CEC_FAILED;
}
