#ifdef __linux__

#ifndef HEADER_CEPOOL
#define HEADER_CEPOOL
#include <map>
#include <thread>
#include <memory>

#include <sys/epoll.h>

#include "EventActions.h"

#define MAX_BUFFER_LEN        8192
class Cevent;
class CEpollImpl;
class CEpoll
{
public:
	CEpoll();
	~CEpoll();

	bool Init(int thread_num = 0);
	bool Dealloc();

	void SyncAccept(std::shared_ptr<CAcceptSocket> sock, const std::function<void(CMemSharePtr<CAcceptEventHandler>&, int error)>& call_back = nullptr);
	void SyncConnect(std::shared_ptr<CSocket> sock, const std::string& ip, short port, const std::function<void(CMemSharePtr<CEventHandler>&, int err)>& call_back = nullptr);

private:
	CAcceptSocket	_accept_socket;
	std::map<std::thread::id, std::shared_ptr<CEpollImpl>>	_epoll_map;
};
#endif
#endif // __linux__
