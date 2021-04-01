#include "rpc_client.h"
#include "info_router.h"
#include "func_thread.h"
#include "parse_package.h"

#include "common/log/log.h"

RPCClient::RPCClient() : _connected(false){
}


RPCClient::~RPCClient() {
}

//start work
void RPCClient::Start(short port, std::string ip) {
    _ip = ip;
    _port = port;
    _net.Init(1);

    _net.SetConnectionCallback(std::bind(&RPCClient::_DoConnect, this, std::placeholders::_1, std::placeholders::_2));
    _net.SetWriteCallback(std::bind(&RPCClient::_DoWrite, this, std::placeholders::_1, std::placeholders::_2));
    _net.SetReadCallback(std::bind(&RPCClient::_DoRead, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    _net.SetDisconnectionCallback(std::bind(&RPCClient::_DoDisConnect, this, std::placeholders::_1, std::placeholders::_2));

    _net.Connection(ip, port);

}

void RPCClient::SetCallBack(const std::string& func_name, Call_back& func) {
	_func_call_map[func_name] = func;
}

void RPCClient::_DoRead(cppnet::Handle handle, cppnet::BufferPtr data,
                         uint32_t len) {
	char recv_buf[8192] = { 0 };
	uint32_t get_len = 8192;
	uint32_t need_len = 0;
	uint32_t recv_len = 0;
	for (;;) {
        get_len = data->ReadUntil(recv_buf, 8192, "\r\n\r\n", strlen("\r\n\r\n"), need_len);
		if (get_len == 0) {
			break;
		}
        std::vector<cppnet::Any> vec;
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

void RPCClient::_DoWrite(cppnet::Handle handle, uint32_t len) {
    // do nothing
}

void RPCClient::_DoConnect(cppnet::Handle handle, uint32_t err) {
    if (err != cppnet::CEC_SUCCESS) {
        cppnet::LOG_ERROR("connect failed! err : %d", err);
        return;
    }
    _socket = handle;
	_connected = true;
    handle->Write("\r\n\r\n", strlen("\r\n\r\n"));
}

void RPCClient::_DoDisConnect(cppnet::Handle handle, uint32_t err) {
	if (err != cppnet::CEC_CLOSED) {
		_net.Connection(_ip, _port);
	}
    cppnet::LOG_ERROR("disconnect with server!");
}
