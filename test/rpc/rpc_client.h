#ifndef TEST_RPC_RPCCLIENT_HEADER
#define TEST_RPC_RPCCLIENT_HEADER

#include "common_struct.h"
#include "parse_package.h"
#include "include/cppnet.h"
#include "include/cppnet_socket.h"
#include "common/util/any.h"

typedef std::function<void(int code, std::vector<cppnet::Any>& ret)> Call_back;

class InfoRouter;
class ParsePackage;
class RPCClient {
public:
    RPCClient();
    ~RPCClient();
    //start work
    void Start(short port, std::string ip);
    //set call back when rpc server response called;
    void SetCallBack(const std::string& func_name, Call_back& func);
    template<typename...Args>
    bool CallFunc(const std::string& func_name, Args&&...args);
public:
    void _DoRead(cppnet::Handle handle, cppnet::BufferPtr data,
                 uint32_t len);
    void _DoWrite(cppnet::Handle handle, uint32_t len);
    void _DoConnect(cppnet::Handle handle, uint32_t err);
    void _DoDisConnect(cppnet::Handle handle, uint32_t err);

private:
    bool                                _connected;
    std::shared_ptr<InfoRouter>            _info_router;
    std::shared_ptr<ParsePackage>        _parse_package;

    cppnet::CppNet                        _net;
    std::string                         _ip;
    int                                 _port;
    cppnet::Handle                      _socket;
    std::map<std::string, Call_back>    _func_call_map;
    std::map<std::string, std::string>  _func_map;
};

template<typename...Args>
bool RPCClient::CallFunc(const std::string& func_name, Args&&...args) {
    if (!_func_map.count(func_name)) {
        return false;
    }
    if (!_func_call_map.count(func_name)) {
        return false;
    }
    if (!_connected) {
        return false;
    }

    std::vector<cppnet::Any> vec;
    _parse_package->ParseParam(vec, std::forward<Args>(args)...);

    char buf[8192] = { 0 };
    int len = 8192;
    if (!_parse_package->PackageFuncCall(buf, len, func_name, _func_map, vec)) {
        return false;
    }
    _socket->Write(buf, len);
    return true;
}

#endif
