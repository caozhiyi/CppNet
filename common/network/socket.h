#ifndef COMMON_NETWORK_SOCKET
#define COMMON_NETWORK_SOCKET

#include <cstdint>

namespace cppnet {

int32_t SocketNoblocking(uint64_t sock);

int32_t ReusePort(uint64_t sock);

// check socket connect
bool CheckConnect(const uint64_t sock);

}
#endif
