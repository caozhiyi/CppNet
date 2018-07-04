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

	void SyncRead();
	void SyncWrite(char* src, int len);

	void SyncRead(unsigned int interval);
	void SyncWrite(unsigned int interval, char* src, int len);

#ifndef __linux__
	void SyncConnection(const std::string& ip, short port, char* buf, int buf_len);
#else
	void SyncConnection(const std::string& ip, short port);
#endif
	void SyncDisconnection();

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
#ifndef __linux__
	//iocp use it save post event num;
	unsigned int					_post_event_num;
#endif
};

#endif