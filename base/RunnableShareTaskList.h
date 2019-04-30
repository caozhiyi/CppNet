#ifndef HEADER_RUNNABLESHARETASKLIST
#define HEADER_RUNNABLESHARETASKLIST

#include <mutex>
#include <map>

#include "TaskQueue.h"

typedef std::function<void()> Task;

template<typename T = Task>
class CRunnableShareTaskList : public CRunnable
{
public:
	explicit CRunnableShareTaskList(int channel);
	virtual ~CRunnableShareTaskList();

	int GetTaskListSize() {
		std::unique_lock<std::mutex> lock(_map_mutex);
		return _task_list_map[_channel].first->Size();
	}

	//post task
	void Push(const T&& func) {
		std::unique_lock<std::mutex> lock(_map_mutex);
		_task_list_map[_channel].first->Push(func);
	}

	void Push(const T& func) {
		std::unique_lock<std::mutex> lock(_map_mutex);
		_task_list_map[_channel].first->Push(func);
	}

	//TO DO
	virtual void Run() = 0;

private:
	T _Pop() {
		return std::move(_task_list_map[_channel].first->Pop());
	}

	CRunnableShareTaskList(const CRunnableShareTaskList&) = delete;
	CRunnableShareTaskList& operator=(const CRunnableShareTaskList&) = delete;

private:
	int								_channel;
	bool							_stop;
	std::shared_ptr<std::thread>	_pthread;

	static	std::mutex															_map_mutex;
	static	std::map<int, std::pair<std::shared_ptr<CTaskQueue<T>>, int>>		_task_list_map;	//shared task queue. channel. TaskQueue. TaskQueue num.
};

template<typename T>
std::mutex CRunnableShareTaskList<T>::_map_mutex;
template<typename T>
std::map<int, std::pair<std::shared_ptr<CTaskQueue<T>>, int>> CRunnableShareTaskList<T>::_task_list_map;

template<typename T>
CRunnableShareTaskList<T>::CRunnableShareTaskList(int channel) : _stop(false), _channel(channel) {
	std::unique_lock<std::mutex> lock(_map_mutex);
	auto iter = _task_list_map.find(_channel);
	if (iter != _task_list_map.end()) {
		iter->second.second++;

	} else {
		_task_list_map[_channel] = std::make_pair(std::shared_ptr<CTaskQueue<Task>>(new CTaskQueue<Task>), 1);
	}
}

template<typename T>
CRunnableShareTaskList<T>::~CRunnableShareTaskList() {
	std::unique_lock<std::mutex> lock(_map_mutex);
	auto iter = _task_list_map.find(_channel);
	if (iter != _task_list_map.end()) {
		iter->second.second--;
		if (iter->second.second == 0) {
			_task_list_map.erase(iter);
		}
	}
}

#endif