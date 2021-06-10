#include <thread>
#include "info_router.h"
#include "func_thread.h"

InfoRouter::InfoRouter() {
}

InfoRouter::~InfoRouter() {
}

void InfoRouter::AddThread(std::shared_ptr<FuncThread>& thread) {
    _func_thread_vec.push_back(thread);
    thread->Start();
}

void InfoRouter::StopAllThread() {
    for (int i = 0; i < (int)_func_thread_vec.size(); i++) {
        _func_thread_vec[i]->Stop();
        _func_thread_vec[i]->Join();
    }
}

void InfoRouter::PushTask(FuncCallInfo* info) {
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _func_thread_vec[_curent_index]->Push(info);
    }
    _curent_index++;
    if (_curent_index > _func_thread_vec.size() - 1) {
        _curent_index = _curent_index % (int)_func_thread_vec.size();
    }
}

void InfoRouter::PushRet(FuncCallInfo* info) {
    _out_task_list.Push(info);
}

FuncCallInfo* InfoRouter::GetRet() {
    return _out_task_list.Pop();
}

void InfoRouter::RegisterFunc(const std::string& name, const CommonFunc& func) {
    std::unique_lock<std::mutex> lock(_mutex);
    for (int i = 0; i < (int)_func_thread_vec.size(); i++) {
        _func_thread_vec[i]->RegisterFunc(name, func);
    }
}

void InfoRouter::RemoveFunc(const std::string& name) {
    std::unique_lock<std::mutex> lock(_mutex);
    for (int i = 0; i < (int)_func_thread_vec.size(); i++) {
        _func_thread_vec[i]->RemoveFunc(name);
    }
}
