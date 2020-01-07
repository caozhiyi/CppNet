#ifndef TEST_RPC_CFUNCTHREAD_HEADER
#define TEST_RPC_CFUNCTHREAD_HEADER

#include <map>
#include "CommonStruct.h"
#include "RunnableAloneTaskList.h"

class CFuncManager;
class CInfoRouter;
class CFuncThread : public base::CRunnableAloneTaskList<FuncCallInfo*>
{
public:
	CFuncThread(std::shared_ptr<CInfoRouter>& router);
    ~CFuncThread();

	//main loop
	virtual void Run();

	virtual void Stop();
	//register function to map
	bool RegisterFunc(const std::string& name, const CommonFunc& func);
	bool RemoveFunc(const std::string& name);

	//find function by name
	CommonFunc FindFunc(const std::string& name);
	//call function by name. Thread unsafety. param_ret use in/out
    bool CallFunc(const std::string& name, std::vector<base::CAny>& param_ret);

private:

    std::mutex                          _mutex;
	std::map<std::string, CommonFunc>	_func_map;
	std::shared_ptr<CInfoRouter>		_func_router;
};

#endif
