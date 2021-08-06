#include "rpc_server.h"
#include "info_router.h"
#include "func_thread.h"
#include "common_struct.h"
#include "parse_package.h"
#include "foundation/log/log.h"
#include "foundation/alloter/pool_alloter.h"

RPCServer::RPCServer():
    _info_router(new InfoRouter), 
    _parse_package(new ParsePackage), 
    _pool(fdan::MakePoolAlloterPtr()), 
    _need_mutex(false){

}

RPCServer::~RPCServer() {

}

void RPCServer::Init(int thread) {
    for (int i = 0; i < thread; i++) {
        auto thread = std::shared_ptr<FuncThread>(new FuncThread(_info_router));
        _info_router->AddThread(thread);
    }
}

void RPCServer::Destroy() {
    _info_router->StopAllThread();
}

void RPCServer::Start(short port, std::string ip) {
    _net.Init(2);

    _net.SetAcceptCallback(std::bind(&RPCServer::_DoAccept, this, std::placeholders::_1, std::placeholders::_2));
    _net.SetWriteCallback(std::bind(&RPCServer::_DoWrite, this, std::placeholders::_1, std::placeholders::_2));
    _net.SetReadCallback(std::bind(&RPCServer::_DoRead, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    _net.ListenAndAccept(ip, port);

    for (;;) {
        auto info = _info_router->GetRet();
        if (info) {
            _PackageAndSend(info->_socket, info, NO_ERROR);
        }
    }

    _net.Join();
}

bool RPCServer::RegisterFunc(std::string name, std::string func_str, const CommonFunc& func) {
    std::unique_lock<std::mutex> lock(_mutex);
    if (_func_map.count(name)) {
        return false;
    }
    _func_map[name] = func_str;
    _info_router->RegisterFunc(name, func);
    return true;
}

bool RPCServer::RemoveFunc(std::string name) {
    std::unique_lock<std::mutex> lock(_mutex);
    if (_func_map.count(name)) {
        _func_map.erase(name);
        _info_router->RemoveFunc(name);
        return true;
    }
    return false;
}

void RPCServer::_DoRead(cppnet::Handle handle, cppnet::BufferPtr data,
                         uint32_t len) {
    uint32_t need_len = 0;
    for (;;) {
        char recv_buf[4096] = { 0 };
        uint32_t read_len = data->ReadUntil(recv_buf, 4096, "\r\n\r\n", strlen("\r\n\r\n"), need_len);
        //get a comlete message
        if (read_len > 0) {
            FuncCallInfo* info = _pool.PoolNew<FuncCallInfo>();
            if (_need_mutex) {
                std::unique_lock<std::mutex> lock(_mutex);
                if (_parse_package->ParseFuncCall(recv_buf + 2, read_len - 2, info->_func_name, _func_map, info->_func_param_ret)) {
                    info->_socket = handle;
                    _info_router->PushTask(info);
                } else {
                    fdan::LOG_ERROR("parse function call request failed!");
                }

            } else {
                if (_parse_package->ParseFuncCall(recv_buf + 2, read_len - 2, info->_func_name, _func_map, info->_func_param_ret)) {
                    info->_socket = handle;
                    _info_router->PushTask(info);
                } else {
                    fdan::LOG_ERROR("parse function call request failed!");
                }
            }

        } else {
            break;
        }
    }
}

void RPCServer::_DoWrite(cppnet::Handle, uint32_t) {
    // do nothing
}

void RPCServer::_DoAccept(cppnet::Handle handle, uint32_t) {
    char buf[8192] = { 0 };
    int len = 8192;

    std::unique_lock<std::mutex> lock(_mutex);
    if (!_parse_package->PackageFuncList(buf, len, _func_map)) {
        fdan::LOG_ERROR("package functnion info failed!");
        abort();
    }
    handle->Write(buf, len);
}

void RPCServer::_PackageAndSend(cppnet::Handle handle, FuncCallInfo* info, int code) {
    if (!info) {
        fdan::LOG_ERROR("function info is null!");
        return;
    }
    bool send = true;
    int get_len = 65535;
    int need_len = 0;
    char send_buf[65535] = { 0 };
    need_len = get_len;
    if (!_parse_package->PackageFuncRet(send_buf, need_len, code, info->_func_name, _func_map, info->_func_param_ret)) {
        fdan::LOG_ERROR("package function response failed!");
        send = false;
    }
    if (send) {
        handle->Write(send_buf, need_len);
    }
}
