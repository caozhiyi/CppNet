#ifdef __linux__
#include <sys/socket.h>
#include "SocketBase.h"
#include "EventActions.h"
#include "MemaryPool.h"

using namespace cppnet;

CSocketBase::CSocketBase() : _add_event_actions(false), _event_actions(nullptr), _pool(new base::CMemoryPool(1024, 20)) {
	memset(_ip, 0, __addr_str_len);
}

CSocketBase::CSocketBase(std::shared_ptr<CEventActions>& event_actions) : _add_event_actions(false), _event_actions(event_actions), _pool(new base::CMemoryPool(1024, 20)) {
	memset(_ip, 0, __addr_str_len);
}

CSocketBase::~CSocketBase() {

}
#endif // __linux__
