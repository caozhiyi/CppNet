#ifndef HEADER_CSOCKETBASE
#define HEADER_CSOCKETBASE

#include <memory>

bool InitScoket();
void DeallocSocket();

static const int __addr_str_len = 16;

class CEventActions;
class CMemaryPool;
class CSocketBase
{
public:
	CSocketBase(std::shared_ptr<CEventActions>& event_actions);
	virtual  ~CSocketBase();

	unsigned int GetSocket() { return _sock; }
	bool IsInActions() { return _add_event_actions; }
	void SetInActions(bool set) { _add_event_actions = set; }
	const char* GetAddress() const { return _ip; }

public:
	bool			_add_event_actions;
	unsigned int	_sock;
	bool			_invalid;
	char			_ip[__addr_str_len];

	std::shared_ptr<CEventActions>	_event_actions;
	std::shared_ptr<CMemaryPool>	_pool;
};

#endif