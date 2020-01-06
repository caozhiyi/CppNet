#ifndef TEST_RPC_CINFOROUTER_HEADER
#define TEST_RPC_CINFOROUTER_HEADER

#include <mutex>
#include <atomic>
#include <memory>
#include <string>
#include <vector>

#include "TaskQueue.h"
#include "CommonStruct.h"

struct FuncCallInfo;
class CFuncThread;
class CInfoRouter
{
public:
	CInfoRouter();
	~CInfoRouter();
	//add a funcmanager thread.
	void AddThread(std::shared_ptr<CFuncThread>& thread);
	void StopAllThread();
	//push call info
	void PushTask(FuncCallInfo* info);
	//push call function return value
	void PushRet(FuncCallInfo* info);
	//may block the thread
	FuncCallInfo* GetRet();

	void RegisterFunc(const std::string& name, const CommonFunc& func);
	void RemoveFunc(const std::string& name);

private:
    base::CTaskQueue<FuncCallInfo*>	            _out_task_list;
	
	std::atomic_int								_curent_index;
	std::mutex									_mutex;
	std::vector<std::shared_ptr<CFuncThread>>	_func_thread_vec;
};

#endif
