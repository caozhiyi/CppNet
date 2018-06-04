#ifndef HEADER_CACCEPTSOCKET
#define HEADER_CACCEPTSOCKET
#include <string>
#include <memory>
#include <functional>

#include "EventHandler.h"
#include "Buffer.h"
#include "PoolSharedPtr.h"
#include "SocketBase.h"

class CMemaryPool;
class CAcceptSocket : public CSocketBase {
public:
	CAcceptSocket(std::shared_ptr<CEventActions>& event_actions);
	~CAcceptSocket();

	bool Bind(short port, const std::string& ip = "");

	bool Listen(unsigned int listen_size);

	void SyncAccept(const std::function<void(CMemSharePtr<CEventHandler>&, int error)>& call_back = nullptr);
	void SyncAccept(const std::function<void(CMemSharePtr<CEventHandler>&, int error)>& accept_back = nullptr,
		const std::function<void(CMemSharePtr<CEventHandler>&, int error)>& read_back = nullptr);

	void SetAcceptCallBack(const std::function<void(CMemSharePtr<CEventHandler>&, int error)>& call_back);
	void SetReadCallBack(const std::function<void(CMemSharePtr<CEventHandler>&, int error)>& call_back);

public:
	void _Accept(CMemSharePtr<CEventHandler>& event);

protected:
	friend class CSocket;

	CMemSharePtr<CEventHandler>		_accept_event;
};

#endif