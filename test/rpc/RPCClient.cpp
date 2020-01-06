#include "Log.h"
#include "CppNet.h"
#include "RPCClient.h"
#include "InfoRouter.h"
#include "FuncThread.h"
#include "ParsePackage.h"

CRPCClient::CRPCClient() : _connected(false){
}


CRPCClient::~CRPCClient() {
}

//start work
void CRPCClient::Start(short port, std::string ip) {
    _ip = ip;
    _port = port;
    cppnet::Init(1);

    cppnet::SetConnectionCallback(std::bind(&CRPCClient::_DoConnect, this, std::placeholders::_1, std::placeholders::_2));
    cppnet::SetWriteCallback(std::bind(&CRPCClient::_DoWrite, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    cppnet::SetReadCallback(std::bind(&CRPCClient::_DoRead, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    cppnet::SetDisconnectionCallback(std::bind(&CRPCClient::_DoDisConnect, this, std::placeholders::_1, std::placeholders::_2));

    cppnet::Connection(ip, port);

}

void CRPCClient::SetCallBack(const std::string& func_name, Call_back& func) {
	_func_call_map[func_name] = func;
}

void CRPCClient::_DoRead(const cppnet::Handle& handle, base::CBuffer* data,
                         uint32_t len, uint32_t err) {
    if (err != cppnet::CEC_SUCCESS) {
        base::LOG_ERROR("read data failed! err : %d", err);
		return;
	}
	char recv_buf[8192] = { 0 };
	int get_len = 8192;
	int need_len = 0;
	int recv_len = 0;
	for (;;) {
        get_len = data->ReadUntil(recv_buf, 8192, "\r\n\r\n", strlen("\r\n\r\n"), need_len);
		if (get_len == 0) {
			break;
		}
        std::vector<base::CAny> vec;
		int type = 0;
		std::string name;
		int code = NO_ERROR;
		if (!_parse_package->ParseType(recv_buf, get_len, type)) {
			if (_func_call_map.count(name)) {
				_func_call_map[name](PARAM_TYPE_ERROR, vec);
			}
			break;
		}
		if (type & FUNCTION_RET) {
			if (!_parse_package->ParseFuncRet(recv_buf + 2, get_len - 2, code, name, _func_map, vec)) {
				if (_func_call_map.count(name)) {
					_func_call_map[name](PARSE_FUNC_ERROR, vec);
				}
				break;
			}
			if (_func_call_map.count(name)) {
				_func_call_map[name](code, vec);
			}

		} else if (type & FUNCTION_INFO) {
			if (!_parse_package->ParseFuncList(recv_buf + 2, get_len - 2, _func_map)) {
				if (_func_call_map.count(name)) {
					_func_call_map[name](PARSE_FUNC_ERROR, vec);
				}
				break;

			} else {
				if (_func_call_map.count(name)) {
					_func_call_map[name](PARAM_TYPE_ERROR, vec);
				}
				break;
			}
		}
    }
}

void CRPCClient::_DoWrite(const cppnet::Handle& handle, uint32_t len, uint32_t err) {
    if (err != cppnet::CEC_SUCCESS) {
        base::LOG_ERROR("send response to client failed! err : %d", err);
	}
}

void CRPCClient::_DoConnect(const cppnet::Handle& handle, uint32_t err) {
    if (err != cppnet::CEC_SUCCESS) {
        base::LOG_ERROR("connect failed! err : %d", err);
        return;
    }
    _socket = handle;
	_connected = true;
    cppnet::Write(handle, "\r\n\r\n", strlen("\r\n\r\n"));
}

void CRPCClient::_DoDisConnect(const cppnet::Handle& handle, uint32_t err) {
    base::LOG_ERROR("disconnect with server!");
    cppnet::Connection(_ip, _port);
}
