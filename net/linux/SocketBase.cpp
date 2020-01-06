#ifdef __linux__
#include <unistd.h>
#include <sys/socket.h>

#include "CNConfig.h"
#include "SocketBase.h"
#include "MemoryPool.h"
#include "EventActions.h"


using namespace cppnet;

CSocketBase::CSocketBase(uint32_t net_index, std::shared_ptr<CallBackHandle>& call_back_handle) : 
                _add_event_actions(false), 
                _net_index(net_index),
                _callback_handle(call_back_handle),
                _event_actions(nullptr), 
                _pool(new base::CMemoryPool(__mem_block_size, __mem_block_add_step)) {
    memset(_ip, 0, __addr_str_len);
}

CSocketBase::CSocketBase(std::shared_ptr<CEventActions>& event_actions, uint32_t net_index, std::shared_ptr<CallBackHandle>& call_back_handle) : 
                _add_event_actions(false), 
                _net_index(net_index),
                _callback_handle(call_back_handle),
                _event_actions(event_actions), 
                _pool(new base::CMemoryPool(__mem_block_size, __mem_block_add_step)) {
    memset(_ip, 0, __addr_str_len);
}

CSocketBase::~CSocketBase() {
    close(_sock);
}
#endif // __linux__
