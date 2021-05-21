// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef CPPNET_SOCKET_POSIX_CONNECT_SOCKET
#define CPPNET_SOCKET_POSIX_CONNECT_SOCKET

#include "cppnet/socket/connect_socket.h"

namespace cppnet {

class WinAcceptEvent;
class WinConnectSocket:
    public ConnectSocket { 
public:
    WinConnectSocket();
    ~WinConnectSocket();

    virtual void Accept();
    void Accept(uint16_t index);

    void OnAccept(std::shared_ptr<WinAcceptEvent> event);

    void SetInActions(bool in) { _in_actions = in; }
    bool GetInActions() { return _in_actions; }

private:
    std::vector<std::shared_ptr<Event>>  _accept_event_vec;
    bool _in_actions;
};

}

#endif