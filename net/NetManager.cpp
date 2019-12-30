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

Handle CCppNetManager::Init(uint32_t thread_num) {
    __cppnet_index++;
    std::shared_ptr<CCppNetImpl> net(new CCppNetImpl(__cppnet_index));
    net->Init(thread_num);
    _cppnet_map[__cppnet_index] = std::move(net);
    return IndexToHandle(__cppnet_index);
}

void CCppNetManager::Dealloc(Handle net_handle) {
    uint32_t index = HandleToIndex(net_handle);
    if (index > __cppnet_index) {
        return;
    }
    _cppnet_map[index]->Dealloc();
}

void CCppNetManager::Join(Handle net_handle) {
    uint32_t index = HandleToIndex(net_handle);
    if (index > __cppnet_index) {
        return;
    }
    _cppnet_map[index]->Join();
}

void CCppNetManager::AllJoin() {
    for (auto iter = _cppnet_map.begin(); iter != _cppnet_map.end(); ++iter) {
        iter->second->Join();
    }
}

void CCppNetManager::SetReadCallback(const read_call_back& func, Handle net_handle) {
    uint32_t index = HandleToIndex(net_handle);
    if (index > __cppnet_index) {
        return;
    }
    _cppnet_map[index]->SetReadCallback(func);
}

void CCppNetManager::SetWriteCallback(const write_call_back& func, Handle net_handle) {
    uint32_t index = HandleToIndex(net_handle);
    if (index > __cppnet_index) {
        return;
    }
    _cppnet_map[index]->SetWriteCallback(func);
}

void CCppNetManager::SetDisconnectionCallback(const connection_call_back& func, Handle net_handle) {
    uint32_t index = HandleToIndex(net_handle);
    if (index > __cppnet_index) {
        return;
    }
    _cppnet_map[index]->SetDisconnectionCallback(func);
}

uint64_t CCppNetManager::SetTimer(uint32_t interval, const std::function<void(void*)>& func, void* param , bool always, Handle net_handle) {
    uint32_t index = HandleToIndex(net_handle);
    if (index > __cppnet_index) {
        return 0;
    }
    return _cppnet_map[index]->SetTimer(interval, func, param, always);
}

void CCppNetManager::RemoveTimer(uint64_t timer_id, Handle net_handle) {
    uint32_t index = HandleToIndex(net_handle);
    if (index > __cppnet_index) {
        return;
    }
    _cppnet_map[index]->RemoveTimer(timer_id);
}

void CCppNetManager::SetAcceptCallback(const connection_call_back& func, Handle net_handle) {
    uint32_t index = HandleToIndex(net_handle);
    if (index > __cppnet_index) {
        return;
    }
    _cppnet_map[index]->SetAcceptCallback(func);
}

bool CCppNetManager::ListenAndAccept(const std::string& ip, uint16_t port, Handle net_handle) {
    uint32_t index = HandleToIndex(net_handle);
    if (index > __cppnet_index) {
        return false;
    }
    return _cppnet_map[index]->ListenAndAccept(ip, port);
}

void CCppNetManager::SetConnectionCallback(const connection_call_back& func, Handle net_handle) {
    uint32_t index = HandleToIndex(net_handle);
    if (index > __cppnet_index) {
        return;
    }
    _cppnet_map[index]->SetConnectionCallback(func);
}

#ifndef __linux__
Handle CCppNetManager::Connection(uint16_t port, std::string ip, const char* buf, uint32_t buf_len, Handle net_handle) {
    uint32_t index = HandleToIndex(net_handle);
    if (index > __cppnet_index) {
        return 0;
    }
    return _cppnet_map[index]->Connection(port, ip, buf, buf_len);
}

#endif
Handle CCppNetManager::Connection(uint16_t port, std::string ip, Handle net_handle) {
    uint32_t index = HandleToIndex(net_handle);
    if (index > __cppnet_index) {
        return 0;
    }
    return _cppnet_map[index]->Connection(port, ip);
}

base::CMemSharePtr<CSocketImpl> CCppNetManager::GetSocket(const Handle& handle) {
    auto index = HandleToIndexAndSocket(handle);
    if (index.first > __cppnet_index) {
        return nullptr;
    }
    return _cppnet_map[index.first]->GetSocket(index.second);
}