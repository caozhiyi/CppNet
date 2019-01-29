#ifndef HEADER_CNETOBJECT
#define HEADER_CNETOBJECT

#include <map>
#include <thread>
#include <mutex>
#include <string>

#include "PoolSharedPtr.h"
#include "EventHandler.h"
#include "MemaryPool.h"
#include "Timer.h"

typedef std::function<void(CMemSharePtr<CSocket>&, int err)> call_back;

class CEventActions;
class CSocket;
class CAcceptSocket;
class CNetObject
{
public:
	CNetObject();
	~CNetObject();
	//common
	void Init(int thread_num);
	void Dealloc();
	void Join();

	void SetReadCallback(const call_back& func);
	void SetWriteCallback(const call_back& func);
	void SetDisconnectionCallback(const call_back& func);
    unsigned int SetTimer(unsigned int interval, const std::function<void(void*)>& func, void* param = nullptr, bool always = false);
    void RemoveTimer(unsigned int timer_id);

	//server
	void SetAcceptCallback(const call_back& func);
	bool ListenAndAccept(int port, std::string ip);

	//client
	void SetConnectionCallback(const call_back& func);

	CMemSharePtr<CSocket> Connection(int port, std::string ip, char* buf, int buf_len);
	CMemSharePtr<CSocket> Connection(int port, std::string ip);

private:
	void _AcceptFunction(CMemSharePtr<CAcceptEventHandler>& event, int err);
	void _ReadFunction(CMemSharePtr<CEventHandler>& event, int err);
	void _WriteFunction(CMemSharePtr<CEventHandler>& event, int err);
	std::shared_ptr<CEventActions>& _RandomGetActions();

private:
	call_back	_read_call_back				= nullptr;
	call_back	_write_call_back			= nullptr;
	call_back	_connection_call_back		= nullptr;
	call_back	_disconnection_call_back	= nullptr;
	call_back	_accept_call_back			= nullptr;
	CMemoryPool			_pool;

	std::mutex			_mutex;
	std::vector<std::shared_ptr<std::thread>>					_thread_vec;
	std::map<unsigned int, CMemSharePtr<CAcceptSocket>>			_accept_socket;
	std::map<unsigned int, CMemSharePtr<CSocket>>				_socket_map;
	std::map<std::thread::id, std::shared_ptr<CEventActions>>	_actions_map;
    std::map<unsigned int, std::weak_ptr<CEventActions>>        _timer_actions_map;
};

#endif