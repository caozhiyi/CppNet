// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#include <fcntl.h>
#include <errno.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/resource.h>

#include "../socket.h"

namespace cppnet {

int32_t SocketNoblocking(uint64_t sock) {
    int32_t old_option = fcntl(sock, F_GETFL);
    int32_t new_option = old_option | O_NONBLOCK;
    fcntl(sock, F_SETFL, new_option);
    return old_option;
}

int32_t ReusePort(uint64_t sock) {
    int32_t opt = 1;
    int32_t ret = setsockopt(sock, SOL_SOCKET, SO_REUSEPORT,
        &opt, static_cast<socklen_t>(sizeof(opt)));
    return ret;
}

bool CheckConnect(const uint64_t sock) {
    struct pollfd fd;
    int32_t ret = 0;
    socklen_t len = 0;
    fd.fd = sock;
    fd.events = POLLOUT;
    if (poll(&fd, 1, -1) == -1) {
        if(errno != EINTR){
            return false;
        }
    }
    len = sizeof(ret);
    if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &ret, &len) == -1) {
        return false;
    }
    if(ret != 0) {
        return false;
    }
    return true;
}


}