#ifndef TEST_RPC_RPCSERVER_HEADER
#define TEST_RPC_RPCSERVER_HEADER

#include <map>
#include <mutex>
#include <memory>
#include <atomic>

#include "common_struct.h"
#include "include/cppnet.h"
#include "foundation/alloter/alloter_interface.h"

class InfoRouter;
class ParsePackage;
class RPCServer {
public:
    RPCServer();
    ~RPCServer();
    //create func thread and add to router
    void Init(int thread);
    //Destroy func thread
    void Destroy();
    //start work
    void Start(short port, std::string ip);

    bool RegisterFunc(std::string name, std::string func_str, const CommonFunc& func);
    bool RemoveFunc(std::string name);

private:
    void _DoRead(cppnet::Handle handle, cppnet::BufferPtr data,
                 uint32_t len);
    void _DoWrite(cppnet::Handle handle, uint32_t len);
    void _DoAccept(cppnet::Handle handle, uint32_t err);
    void _PackageAndSend(cppnet::Handle handle, FuncCallInfo* info, int code);

private:
    std::shared_ptr<InfoRouter>            _info_router;
    std::shared_ptr<ParsePackage>        _parse_package;

    cppnet::CppNet                        _net;
    fdan::AlloterWrap                 _pool;
    std::atomic_bool                    _need_mutex;
    std::mutex                            _mutex;
    std::map<std::string, std::string>    _func_map;
};

#endif
