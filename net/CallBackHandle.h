#ifndef HEADER_NET_CALLBACKHANDLE
#define HEADER_NET_CALLBACKHANDLE

#include "PoolSharedPtr.h"

namespace cppnet {
    class CEventHandler;
    class CSocketImpl;
    struct CallBackHandle {
        std::function<void(base::CMemSharePtr<CSocketImpl>&, uint32_t)> _accept_call_back;
        std::function<void(base::CMemSharePtr<CEventHandler>&, uint32_t)> _read_call_back;
        std::function<void(base::CMemSharePtr<CEventHandler>&, uint32_t)> _write_call_back;
    };
}

#endif