#ifndef HEADER_NET_NETHANDLE
#define HEADER_NET_NETHANDLE

#include "Type.h"

namespace cppnet {

    union NetHandle {
        struct Field {
            uint32_t    _socket;
            uint32_t    _cppnet_index;
        } _field;
        uint64_t        _cppnet_handle;
    };

    static uint32_t HandleToSocket(uint64_t net_handle) {
        NetHandle handle;
        handle._cppnet_handle = net_handle;
        return handle._field._socket;
    }

    static uint32_t HandleToIndex(uint64_t net_handle) {
        NetHandle handle;
        handle._cppnet_handle = net_handle;
        return handle._field._cppnet_index;
    }

    // return cppnet index, socket
    static std::pair<uint32_t, uint32_t> HandleToIndexAndSocket(uint64_t net_handle) {
        NetHandle handle;
        handle._cppnet_handle = net_handle;
        return std::make_pair(handle._field._cppnet_index, handle._field._socket);
    }

    static uint64_t SocketToHandle(uint32_t sock) {
        NetHandle handle;
        handle._field._socket= sock;
        return handle._cppnet_handle;
    }

    static uint64_t IndexToHandle(uint32_t index) {
        NetHandle handle;
        handle._field._cppnet_index= index;
        return handle._cppnet_handle;
    }

    static uint64_t IndexAndSocketToHandle(uint32_t index, uint32_t sock) {
        NetHandle handle;
        handle._field._socket= sock;
        handle._field._cppnet_index= index;
        return handle._cppnet_handle;
    }
}
#endif