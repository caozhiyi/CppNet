#ifndef HEADER_RUNNABLEALONETASKLISTWITHPOST
#define HEADER_RUNNABLEALONETASKLISTWITHPOST

#include <mutex>
#include <map>

#include "Runnable.h"
#include "TaskQueue.h"

typedef std::function<void()> Task;

class CRunnableAloneTaskListWithPost : public CRunnable
{
public:
	CRunnableAloneTaskListWithPost() {}
	virtual ~CRunnableAloneTaskListWithPost() {}

	int GetTaskListSize() {
		return _task_list.Size();
	}

	virtual void Start();
	virtual void Stop();

	//post task
	void Push(const Task&& func) {
		_task_list.Push(func);
	}
	void Push(const Task& func) {
		_task_list.Push(func);
	}

	//TO DO
	virtual void Run();

	std::thread::id GetId()const { return _id; }

	//post task to the thread
	static	bool PostTask(const std::thread::id& thread_id, const Task& func);

private:
	Task _Pop() {
		return std::move(_task_list.Pop());
	}

	CRunnableAloneTaskListWithPost(const CRunnableAloneTaskListWithPost&) = delete;
	CRunnableAloneTaskListWithPost& operator=(const CRunnableAloneTaskListWithPost&) = delete;

private:
	CTaskQueue<Task>		_task_list;	
	std::thread::id			_id;

	static	std::mutex													_map_mutex;	
	static	std::map<std::thread::id, CRunnableAloneTaskListWithPost*>	_runnable_map;
};

std::mutex CRunnableAloneTaskListWithPost::_map_mutex;
std::map<std::thread::id, CRunnableAloneTaskListWithPost*> CRunnableAloneTaskListWithPost::_runnable_map;

void CRunnableAloneTaskListWithPost::Start() {
	_stop = false;
	if (!_pthread) {
		_pthread = std::shared_ptr<std::thread>(new std::thread(std::bind(&CRunnableAloneTaskListWithPost::Run, this)));
	}

	Push([this]() {
		std::unique_lock<std::mutex> lock(_map_mutex);
		auto iter = _runnable_map.find(std::this_thread::get_id());
		if (iter == _runnable_map.end()) {
			_runnable_map[std::this_thread::get_id()] = this;
			_id = std::this_thread::get_id();
		}
	});
}

void CRunnableAloneTaskListWithPost::Stop() {
	Push([this]() {
		{
			std::unique_lock<std::mutex> lock(_map_mutex);
			auto iter = _runnable_map.find(std::this_thread::get_id());
			if (iter != _runnable_map.end()) {
				_runnable_map.erase(iter);
			}
		}
		Push(nullptr);
		_stop = true;
	});
}

bool CRunnableAloneTaskListWithPost::PostTask(const std::thread::id& thread_id, const Task& func) {
	std::unique_lock<std::mutex> lock(_map_mutex);
	auto iter = _runnable_map.find(thread_id);
	if (iter != _runnable_map.end()) {
		if (iter->second) {
			iter->second->Push(func);
			return true;
		}
	}
	return false;
}

void  CRunnableAloneTaskListWithPost::Run() {
	while (!_stop) {
		auto t = _Pop();
		if (t) {
			t();

		} else {
			break;
		}
	}
}

#endif