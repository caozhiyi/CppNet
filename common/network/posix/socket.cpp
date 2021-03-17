#if ((defined __linux__) || (defined __APPLE__)) 

#include <fcntl.h>
#include <sys/socket.h>
#include <sys/resource.h>

#include "socket.h"

namespace cppnet {

int32_t SocketNoblocking(uint64_t sock) {
    int old_option = fcntl(sock, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(sock, F_SETFL, new_option);
    return old_option;
}

int32_t ReusePort(uint64_t sock) {
    int opt = 1;
    int ret = setsockopt(sock, SOL_SOCKET, SO_REUSEPORT,
        &opt, static_cast<socklen_t>(sizeof(opt)));
    return ret;
}

}

#endif