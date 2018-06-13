#ifdef __linux__
#include <unistd.h>
#include "SocketBase.h"
#include "Log.h"

CSocketBase::CSocketBase(std::shared_ptr<CEventActions>& event_actions) : _add_event_actions(false), _invalid(false), _event_actions(event_actions), _pool(new CMemaryPool(1024, 20)) {
	memset(_ip, 0, __addr_str_len);
}

CSocketBase::~CSocketBase() {
	if (event_actions) {
		event_actions->DelEvent(_sock);
	}
	close(_sock);
}
#endif // __linux__
