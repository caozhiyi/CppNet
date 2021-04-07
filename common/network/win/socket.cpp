// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#include <winsock2.h>
#include <Mswsock.h >
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
    int32_t seconds;
    int32_t bytes = sizeof(seconds);
    int32_t iResult = 0;

    iResult = getsockopt(sock, SOL_SOCKET, SO_CONNECT_TIME, (char*)&seconds, (PINT)&bytes);
    if (iResult != NO_ERROR) {
        return false;

    } else {
        if (seconds == 0xFFFFFFFF) {
            return false;
        }
    }
    return true;
}


}