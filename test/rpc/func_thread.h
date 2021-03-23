#ifndef TEST_RPC_CFUNCTHREAD_HEADER
#define TEST_RPC_CFUNCTHREAD_HEADER

#include <map>
#include "common_struct.h"
#include "common/util/any.h"
#include "common/thread/thread_with_queue.h"

class FuncManager;
class InfoRouter;
class FuncThread : public cppnet::ThreadWithQueue<FuncCallInfo*> {
public:
	FuncThread(std::shared_ptr<InfoRouter>& router);
    ~FuncThread();

	//main loop
	virtual void Run();

	virtual void Stop();
	//register function to map
	bool RegisterFunc(const std::string& name, const CommonFunc& func);
	bool RemoveFunc(const std::string& name);

	//find function by name
	CommonFunc FindFunc(const std::string& name);
	//call function by name. Thread unsafety. param_ret use in/out
    bool CallFunc(const std::string& name, std::vector<cppnet::Any>& param_ret);

private:

    std::mutex                          _mutex;
	std::map<std::string, CommonFunc>	_func_map;
	std::shared_ptr<InfoRouter>		    _func_router;
};

#endif
