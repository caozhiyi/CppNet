#ifdef __linux__
#include <sys/socket.h>
#include <unistd.h>
#include "SocketBase.h"
#include "EventActions.h"
#include "MemaryPool.h"

const uint16_t __mem_block_size     = 1024;
const uint16_t __mem_block_add_step = 5;

using namespace cppnet;

CSocketBase::CSocketBase() : _add_event_actions(false), _event_actions(nullptr), 
                             _pool(new base::CMemoryPool(__mem_block_size, __mem_block_add_step)) {
    memset(_ip, 0, __addr_str_len);
}

CSocketBase::CSocketBase(std::shared_ptr<CEventActions>& event_actions) : _add_event_actions(false), _event_actions(event_actions), 
                             _pool(new base::CMemoryPool(__mem_block_size, __mem_block_add_step)) {
    memset(_ip, 0, __addr_str_len);
}

CSocketBase::~CSocketBase() {
    close(_sock);
}
#endif // __linux__
