#ifndef TEST_RPC_RPCCLIENT_HEADER
#define TEST_RPC_RPCCLIENT_HEADER

#include <memory>

#include <iostream>

#include "CppDefine.h"
#include "Socket.h"
#include "CommonStruct.h"
#include "ParsePackage.h"

typedef std::function<void(int code, std::vector<base::CAny>& ret)> Call_back;

class CInfoRouter;
class CParsePackage;
class CRPCClient {
public:
	CRPCClient();
	~CRPCClient();
	//start work
	void Start(short port, std::string ip);
	//set call back when rpc server response called;
	void SetCallBack(const std::string& func_name, Call_back& func);
	template<typename...Args>
	bool CallFunc(const std::string& func_name, Args&&...args);
public:
    void _DoRead(const cppnet::Handle& handle, base::CBuffer* data,
                 uint32_t len, uint32_t err);
    void _DoWrite(const cppnet::Handle& handle, uint32_t len, uint32_t err);
    void _DoConnect(const cppnet::Handle& handle, uint32_t err);
    void _DoDisConnect(const cppnet::Handle& handle, uint32_t err);

private:
    bool				                _connected;
	std::shared_ptr<CInfoRouter>		_info_router;
	std::shared_ptr<CParsePackage>		_parse_package;

    std::string                         _ip;
    int                                 _port;
    cppnet::Handle                      _socket;
	std::map<std::string, Call_back>	_func_call_map;
	std::map<std::string, std::string>  _func_map;
};

template<typename...Args>
bool CRPCClient::CallFunc(const std::string& func_name, Args&&...args) {
	if (!_func_map.count(func_name)) {
		return false;
	}
	if (!_func_call_map.count(func_name)) {
		return false;
	}
	if (!_connected) {
		return false;
	}

    std::vector<base::CAny> vec;
	_parse_package->ParseParam(vec, std::forward<Args>(args)...);

	char buf[8192] = { 0 };
	int len = 8192;
	if (!_parse_package->PackageFuncCall(buf, len, func_name, _func_map, vec)) {
		return false;
	}
    cppnet::Write(_socket, buf, len);
    return true;
}

#endif
