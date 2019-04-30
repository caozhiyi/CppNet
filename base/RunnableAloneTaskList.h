#ifndef HEADER_RUNNABLEALONETASKLIST
#define HEADER_RUNNABLEALONETASKLIST

#include "Runnable.h"
#include "TaskQueue.h"

typedef std::function<void()> Task;

template<typename T = Task>
class CRunnableAloneTaskList : public CRunnable
{
public:
	CRunnableAloneTaskList() {}
	virtual ~CRunnableAloneTaskList() {}

	int GetTaskListSize() {
		return _task_list.Size();
	}

	//post task
	void Push(const T&& t) {
		_task_list.Push(t);
	}
	void Push(const T& t) {
		_task_list.Push(t);
	}

	//TO DO
	virtual void Run() = 0;

protected:
	T _Pop() {
		return std::move(_task_list.Pop());
	}

	CRunnableAloneTaskList(const CRunnableAloneTaskList&) = delete;
	CRunnableAloneTaskList& operator=(const CRunnableAloneTaskList&) = delete;

private:
	CTaskQueue<T>			_task_list;			//every thread have a task queue
};
#endif