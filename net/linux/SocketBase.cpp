#ifdef __linux__
#include "SocketBase.h"
#include "EventActions.h"

using namespace cppnet;

CSocketBase::CSocketBase() : _add_event_actions(false), _invalid(false), _event_actions(nullptr), _pool(new CMemoryPool(1024, 20)) {
	memset(_ip, 0, __addr_str_len);
}

CSocketBase::CSocketBase(std::shared_ptr<CEventActions>& event_actions) : _add_event_actions(false), _invalid(false), _event_actions(event_actions), _pool(new CMemoryPool(1024, 20)) {
	memset(_ip, 0, __addr_str_len);
}

CSocketBase::~CSocketBase() {

}
#endif // __linux__
