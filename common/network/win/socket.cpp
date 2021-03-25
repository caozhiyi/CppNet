#include <winsock2.h>
#include "../socket.h"

namespace cppnet {

int32_t SocketNoblocking(uint64_t sock) {
    return 0;
}

int32_t ReusePort(uint64_t sock) {
    int32_t opt = 1;
    return setsockopt((SOCKET)sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
}

bool CheckConnect(const uint64_t sock) {
    return false;
}


}