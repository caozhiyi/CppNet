#ifndef HEADER_CSOCKET
#define HEADER_CSOCKET

#include <string>
#include <memory>
#include <functional>

#include "Buffer.h"
#include "PoolSharedPtr.h"
#include "SocketBase.h"

class CEventHandler;
class CBuffer;
class CSocket : public CSocketBase, public CEnableSharedFromThis<CSocket> {
public:
	CSocket(std::shared_ptr<CEventActions>& event_actions);
	~CSocket();

	void SyncRead(const std::function<void(CMemSharePtr<CEventHandler>&, int err)>& call_back = nullptr);
	void SyncWrite(char* src, int len, const std::function<void(CMemSharePtr<CEventHandler>&, int err)>& call_back = nullptr);

	void SyncRead(unsigned int interval, const std::function<void(CMemSharePtr<CEventHandler>&, int err)>& call_back = nullptr);
	void SyncWrite(unsigned int interval, char* src, int len, const std::function<void(CMemSharePtr<CEventHandler>&, int error)>& call_back = nullptr);

	void SyncConnection(const std::string& ip, short port, const std::function<void(CMemSharePtr<CEventHandler>&, int err)>& call_back = nullptr);
	void SyncDisconnection(const std::function<void(CMemSharePtr<CEventHandler>&, int err)>& call_back = nullptr);

	void SetReadCallBack(const std::function<void(CMemSharePtr<CEventHandler>&, int error)>& call_back);
	void SetWriteCallBack(const std::function<void(CMemSharePtr<CEventHandler>&, int error)>& call_back);

	friend bool operator>(const CSocketBase& s1, const CSocketBase& s2);
	friend bool operator<(const CSocketBase& s1, const CSocketBase& s2);
	friend bool operator==(const CSocketBase& s1, const CSocketBase& s2);
	friend bool operator!=(const CSocketBase& s1, const CSocketBase& s2);

public:
	void _Recv(CMemSharePtr<CEventHandler>& event);
	void _Send(CMemSharePtr<CEventHandler>& event);

public:
	CMemSharePtr<CEventHandler>		_read_event;
	CMemSharePtr<CEventHandler>		_write_event;
	//iocp use it
	unsigned int					_post_event_num;
};

#endif