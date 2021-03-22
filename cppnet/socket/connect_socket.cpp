#include <string>
#include <errno.h>

#include "rw_socket.h"
#include "connect_socket.h"
#include "common/log/log.h"
#include "common/network/socket.h"
#include "common/network/address.h"
#include "common/network/io_handle.h"
#include "common/alloter/pool_alloter.h"

#include "cppnet/cppnet_base.h"
#include "cppnet/cppnet_config.h"
#include "cppnet/event/event_interface.h"
#include "cppnet/event/action_interface.h"

namespace cppnet {

ConnectSocket::ConnectSocket() {

}

ConnectSocket::~ConnectSocket() {

}

bool ConnectSocket::Bind(const std::string& ip, uint16_t port) {
    if (_sock == 0) {
        auto ret = OsHandle::TcpSocket();
        if (ret._return_value < 0) {
            LOG_ERROR("create socket failed. errno:%d", ret._errno);
            return false;
        }
        _sock = ret._return_value;
    }

    _addr = std::make_shared<Address>(AT_IPV4, ip, port);

    auto ret = OsHandle::Bind(_sock, *_addr);

    if (ret._return_value < 0) {
        LOG_FATAL("linux bind socket filed! error code:%d", ret._errno);
        OsHandle::Close(_sock);
        return false;
    }

    return true;
}

bool ConnectSocket::Listen() {
    auto ret = OsHandle::Listen(_sock);
    if (ret._return_value < 0) {
        LOG_FATAL("linux listen socket filed! error code:%d", ret._errno);
        OsHandle::Close(_sock);
        return false;
    }

    //set the socket noblocking
    SocketNoblocking(_sock);

    Accept();

    return true;
}

void ConnectSocket::Accept() {
    if (!_read_event) {
        _read_event = std::make_shared<Event>();
        _read_event->SetSocket(shared_from_this());
    }
    __all_socket_map[_sock] = shared_from_this();
    auto actions = GetEventActions();
    if (actions) {
        actions->AddAcceptEvent(_read_event);
    }
}

void ConnectSocket::OnAccept() {
    while (true) {
        std::shared_ptr<AlloterWrap> alloter = std::make_shared<AlloterWrap>(MakePoolAlloterPtr());
        std::shared_ptr<Address> address = alloter->PoolNewSharePtr<Address>(AT_IPV4);
        //may get more than one connections
        auto ret = OsHandle::Accept(_sock, *address);
        if (ret._return_value < 0) {
            if (errno == EAGAIN) {
                break;
            }
            LOG_FATAL("accept socket filed! error code:%d", ret._errno);
            break;
        }

        auto cppnet_base = _cppnet_base.lock();
        if (!cppnet_base) {
            return;
        }

        //set the socket noblocking
        SocketNoblocking(ret._return_value);
        
        //create a new socket.
        auto sock = std::make_shared<RWSocket>(ret._return_value, alloter);

        sock->SetCppNetBase(cppnet_base);
        sock->SetEventActions(_event_actions);
        sock->SetAddress(address);

        __all_socket_map[ret._return_value] = sock;

        //call accept call back function
        cppnet_base->OnAccept(sock);

        //start read
        sock->Read();
    }
}

}