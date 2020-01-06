#include "Log.h"
#include "CppNet.h"
#include "RPCServer.h"
#include "InfoRouter.h"
#include "FuncThread.h"
#include "CommonStruct.h"
#include "ParsePackage.h"

CRPCServer::CRPCServer() : _info_router(new CInfoRouter), _parse_package(new CParsePackage), _pool(1024, 5), _need_mutex(false){

}

CRPCServer::~CRPCServer() {

}

void CRPCServer::Init(int thread) {
	for (int i = 0; i < thread; i++) {
		auto thread = std::shared_ptr<CFuncThread>(new CFuncThread(_info_router));
		_info_router->AddThread(thread);
	}
}

void CRPCServer::Destroy() {
	_info_router->StopAllThread();
}

void CRPCServer::Start(short port, std::string ip) {
    cppnet::Init(2);

    cppnet::SetAcceptCallback(std::bind(&CRPCServer::_DoAccept, this, std::placeholders::_1, std::placeholders::_2));
    cppnet::SetWriteCallback(std::bind(&CRPCServer::_DoWrite, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    cppnet::SetReadCallback(std::bind(&CRPCServer::_DoRead, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

    cppnet::ListenAndAccept(ip, port);

	for (;;) {
		auto info = _info_router->GetRet();
        if (info) {
            _PackageAndSend(info->_socket, info, NO_ERROR);
		}
	}

    cppnet::Join();
}

bool CRPCServer::RegisterFunc(std::string name, std::string func_str, const CommonFunc& func) {
	std::unique_lock<std::mutex> lock(_mutex);
    if (_func_map.count(name)) {
		return false;
	}
	_func_map[name] = func_str;
    _info_router->RegisterFunc(name, func);
	return true;
}

bool CRPCServer::RemoveFunc(std::string name) {
	std::unique_lock<std::mutex> lock(_mutex);
	if (_func_map.count(name)) {
		_func_map.erase(name);
        _info_router->RemoveFunc(name);
		return true;
    }
	return false;
}

void CRPCServer::_DoRead(const cppnet::Handle& handle, base::CBuffer* data,
                         uint32_t len, uint32_t err) {
    if (err != cppnet::CEC_SUCCESS) {
        base::LOG_ERROR("read data failed! err : %d", err);
		return;
	}
	int need_len = 0;
	for (;;) {
        char recv_buf[4096] = { 0 };
        int read_len = data->ReadUntil(recv_buf, 4096, "\r\n\r\n", strlen("\r\n\r\n"), need_len);
		//get a comlete message
		if (read_len > 0) {
            FuncCallInfo* info = _pool.PoolNew<FuncCallInfo>();
			if (_need_mutex) {
				std::unique_lock<std::mutex> lock(_mutex);
				if (_parse_package->ParseFuncCall(recv_buf + 2, read_len - 2, info->_func_name, _func_map, info->_func_param_ret)) {
                    info->_socket = handle;
					_info_router->PushTask(info);
				} else {
                    base::LOG_ERROR("parse function call request failed!");
				}

			} else {
				if (_parse_package->ParseFuncCall(recv_buf + 2, read_len - 2, info->_func_name, _func_map, info->_func_param_ret)) {
                    info->_socket = handle;
					_info_router->PushTask(info);
				} else {
                    base::LOG_ERROR("parse function call request failed!");
				}
            }

        } else {
			break;
		}
    }
}

void CRPCServer::_DoWrite(const cppnet::Handle&, uint32_t, uint32_t err) {
    if (err != cppnet::CEC_SUCCESS) {
        base::LOG_ERROR("send response to client failed!");
	}
}

void CRPCServer::_DoAccept(const cppnet::Handle& handle, uint32_t) {
	char buf[8192] = { 0 };
	int len = 8192;

    std::unique_lock<std::mutex> lock(_mutex);
    if (!_parse_package->PackageFuncList(buf, len, _func_map)) {
        base::LOG_ERROR("package functnion info failed!");
        abort();
    }
    base::LOG_DEBUG("send to %d, buf : %s", handle, buf);
    cppnet::Write(handle, buf, len);
}

void CRPCServer::_PackageAndSend(const cppnet::Handle& handle, FuncCallInfo* info, int code) {
	if (!info) {
        base::LOG_ERROR("function info is null!");
		return;
	}
	bool send = true;
	int get_len = 65535;
	int need_len = 0;
    char send_buf[65535] = { 0 };
	need_len = get_len;
	if (!_parse_package->PackageFuncRet(send_buf, need_len, code, info->_func_name, _func_map, info->_func_param_ret)) {
        base::LOG_ERROR("package function response failed!");
        send = false;
	}
	if (send) {
        cppnet::Write(handle, send_buf, need_len);
    }
}
