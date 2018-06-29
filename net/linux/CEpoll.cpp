#ifdef __linux__
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "CEpoll.h"
#include "OSInfo.h"
#include "Log.h"
#include "EventHandler.h"
#include "Buffer.h"
#include "Socket.h"
#include "Timer.h"
#include "LinuxFunc.h"
#include "EpollImpl.h"

CEpoll::CEpoll() {

}

CEpoll::~CEpoll() {

}

bool CEpoll::Init(int thread_num = 0) {
	//make coredump file
	SetCoreFileUnlimit();

	int cpus = GetCpuNum();
	if (thread_num < 0 || thread_num > cpus * 2) {
		thread_num = cpus;
	}

	for (int i = 0; i < thread_num; i++) {
		std::shared_ptr<CEpollImpl> epoll(new CEpollImpl);
		std::thread thd(std::thread(std::bind(&CEpollImpl::ProcessEvent, epoll)));
		_epoll_map[thd->get_id()] = epoll;
	}

	LOG_DEBUG("epoll init success, %d", errno);
	return true;
}

bool CEpoll::Dealloc() {
	for (auto iter = _epoll_map.begin(); iter < _epoll_map.end(); ++iter) {
		iter->second->Dealloc();
	}
	_epoll_map.clear();
	LOG_DEBUG("epoll close success, %d", errno);
	return true;
}

void SyncAccept(std::shared_ptr<CAcceptSocket> sock, const std::function<void(CMemSharePtr<CAcceptEventHandler>&, int error)>& call_back) {

}

void SyncConnect(std::shared_ptr<CSocket> sock, const std::string& ip, short port, const std::function<void(CMemSharePtr<CEventHandler>&, int err)>& call_back) {

}

#endif // __linux__
