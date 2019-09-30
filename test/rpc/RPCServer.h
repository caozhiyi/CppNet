#ifndef TEST_RPC_RPCSERVER_HEADER
#define TEST_RPC_RPCSERVER_HEADER
#include <memory>
#include <atomic>
#include <map>

#include "MemoryPool.h"
#include "CommonStruct.h"
#include "CppDefine.h"

class CInfoRouter;
class CParsePackage;
class CRPCServer {
public:
	CRPCServer();
	~CRPCServer();
	//create func thread and add to router
	void Init(int thread);
	//Destroy func thread
	void Destroy();
	//start work
	void Start(short port, std::string ip);

	bool RegisterFunc(std::string name, std::string func_str, const CommonFunc& func);
	bool RemoveFunc(std::string name);

private:
    void _DoRead(const cppnet::Handle& handle, base::CBuffer* data,
                 uint32_t len, uint32_t err);
    void _DoWrite(const cppnet::Handle& handle, uint32_t len, uint32_t err);
    void _DoAccept(const cppnet::Handle& handle, uint32_t err);
    void _PackageAndSend(const cppnet::Handle& handle, FuncCallInfo* info, int code);

private:
	std::shared_ptr<CInfoRouter>		_info_router;
	std::shared_ptr<CParsePackage>		_parse_package;

    base::CMemoryPool                   _pool;
	std::atomic_bool					_need_mutex;
	std::mutex							_mutex;
	std::map<std::string, std::string>	_func_map;
};

#endif
