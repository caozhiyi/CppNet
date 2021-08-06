#ifndef TEST_RPC_CINFOROUTER_HEADER
#define TEST_RPC_CINFOROUTER_HEADER

#include <mutex>
#include <atomic>
#include <memory>
#include <string>
#include <vector>

#include "common_struct.h"
#include "foundation/structure/thread_safe_block_queue.h"

struct FuncCallInfo;
class FuncThread;
class InfoRouter {
public:
    InfoRouter();
    ~InfoRouter();
    //add a funcmanager thread.
    void AddThread(std::shared_ptr<FuncThread>& thread);
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
    fdan::ThreadSafeBlockQueue<FuncCallInfo*> _out_task_list;
    
    std::atomic_int                                _curent_index;
    std::mutex                                    _mutex;
    std::vector<std::shared_ptr<FuncThread>>    _func_thread_vec;
};

#endif
