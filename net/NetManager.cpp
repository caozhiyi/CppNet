#include "NetManager.h"
#include "CppNetImpl.h"
#include "NetHandle.h"

using namespace cppnet;

static uint32_t __cppnet_index = 0;

CCppNetManager::CCppNetManager() {

}

CCppNetManager::~CCppNetManager() {
    for (auto iter = _cppnet_map.begin(); iter != _cppnet_map.end(); ++iter) {
        iter->second->Dealloc();
    }
}

int32_t CCppNetManager::Init(uint32_t thread_num) {
    __cppnet_index++;
    std::shared_ptr<CCppNetImpl> net(new CCppNetImpl(__cppnet_index));
    net->Init(thread_num);
    _cppnet_map[__cppnet_index] = std::move(net);
    return __cppnet_index;
}

void CCppNetManager::Dealloc(int32_t net_handle) {
    if (net_handle > __cppnet_index) {
        return;
    }
    _cppnet_map[net_handle]->Dealloc();
}

void CCppNetManager::Join(int32_t net_handle) {
    if (net_handle > __cppnet_index) {
        return;
    }
    _cppnet_map[net_handle]->Join();
}

void CCppNetManager::AllJoin() {
    for (auto iter = _cppnet_map.begin(); iter != _cppnet_map.end(); ++iter) {
        iter->second->Join();
    }
}

void CCppNetManager::SetReadCallback(const read_call_back& func, int32_t net_handle) {
    if (net_handle > __cppnet_index) {
        return;
    }
    _cppnet_map[net_handle]->SetReadCallback(func);
}

void CCppNetManager::SetWriteCallback(const write_call_back& func, int32_t net_handle) {
    if (net_handle > __cppnet_index) {
        return;
    }
    _cppnet_map[net_handle]->SetWriteCallback(func);
}

void CCppNetManager::SetDisconnectionCallback(const connection_call_back& func, int32_t net_handle) {
    if (net_handle > __cppnet_index) {
        return;
    }
    _cppnet_map[net_handle]->SetDisconnectionCallback(func);
}

uint64_t CCppNetManager::SetTimer(uint32_t interval, const std::function<void(void*)>& func, void* param , bool always, int32_t net_handle) {
    if (net_handle > __cppnet_index) {
        return 0;
    }
    return _cppnet_map[net_handle]->SetTimer(interval, func, param, always);
}

void CCppNetManager::RemoveTimer(uint64_t timer_id, int32_t net_handle) {
    if (net_handle > __cppnet_index) {
        return;
    }
    _cppnet_map[net_handle]->RemoveTimer(timer_id);
}

void CCppNetManager::SetAcceptCallback(const connection_call_back& func, int32_t net_handle) {
    if (net_handle > __cppnet_index) {
        return;
    }
    _cppnet_map[net_handle]->SetAcceptCallback(func);
}

bool CCppNetManager::ListenAndAccept(const std::string& ip, uint16_t port, int32_t net_handle) {
    if (net_handle > __cppnet_index) {
        return false;
    }
    return _cppnet_map[net_handle]->ListenAndAccept(ip, port);
}

void CCppNetManager::SetConnectionCallback(const connection_call_back& func, int32_t net_handle) {
    if (net_handle > __cppnet_index) {
        return;
    }
    _cppnet_map[net_handle]->SetConnectionCallback(func);
}

#ifndef __linux__
Handle CCppNetManager::Connection(uint16_t port, std::string ip, const char* buf, uint32_t buf_len, int32_t net_handle) {
    if (net_handle > __cppnet_index) {
        return 0;
    }
    return _cppnet_map[net_handle]->Connection(port, ip, buf, buf_len);
}

#endif
Handle CCppNetManager::Connection(uint16_t port, std::string ip, int32_t net_handle) {
    if (net_handle > __cppnet_index) {
        return 0;
    }
    return _cppnet_map[net_handle]->Connection(port, ip);
}

base::CMemSharePtr<CSocketImpl> CCppNetManager::GetSocket(const Handle& handle) {
    auto index = HandleToIndexAndSocket(handle);
    if (index.first > __cppnet_index) {
        return nullptr;
    }
    return _cppnet_map[index.first]->GetSocket(index.second);
}