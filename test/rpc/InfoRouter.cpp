#include <thread>
#include "InfoRouter.h"
#include "FuncThread.h"

CInfoRouter::CInfoRouter() {
}

CInfoRouter::~CInfoRouter() {
}

void CInfoRouter::AddThread(std::shared_ptr<CFuncThread>& thread) {
	_func_thread_vec.push_back(thread);
	thread->Start();
}

void CInfoRouter::StopAllThread() {
	for (int i = 0; i < (int)_func_thread_vec.size(); i++) {
		_func_thread_vec[i]->Stop();
        _func_thread_vec[i]->Join();
	}
}

void CInfoRouter::PushTask(FuncCallInfo* info) {
	{
		std::unique_lock<std::mutex> lock(_mutex);
		_func_thread_vec[_curent_index]->Push(info);
	}
	_curent_index++;
	if (_curent_index > _func_thread_vec.size() - 1) {
		_curent_index = _curent_index % (int)_func_thread_vec.size();
	}
}

void CInfoRouter::PushRet(FuncCallInfo* info) {
	_out_task_list.Push(info);
}

FuncCallInfo* CInfoRouter::GetRet() {
	return _out_task_list.Pop();
}

void CInfoRouter::RegisterFunc(const std::string& name, const CommonFunc& func) {
    std::unique_lock<std::mutex> lock(_mutex);
	for (int i = 0; i < (int)_func_thread_vec.size(); i++) {
		_func_thread_vec[i]->RegisterFunc(name, func);
	}
}

void CInfoRouter::RemoveFunc(const std::string& name) {
    std::unique_lock<std::mutex> lock(_mutex);
	for (int i = 0; i < (int)_func_thread_vec.size(); i++) {
		_func_thread_vec[i]->RemoveFunc(name);
	}
}
