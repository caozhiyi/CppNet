// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)
// Copyright <caozhiyi5@gmail.com>

#include <errno.h>

#include <string>
#include <utility>

#include "cppnet/cppnet_base.h"
#include "cppnet/cppnet_config.h"
#include "cppnet/socket/rw_socket.h"
#include "cppnet/event/event_interface.h"
#include "cppnet/socket/connect_socket.h"
#include "cppnet/event/action_interface.h"

#include "foundation/log/log.h"
#include "foundation/os/convert.h"
#include "foundation/network/socket.h"
#include "foundation/network/address.h"
#include "foundation/network/net_handle.h"
#include "foundation/alloter/pool_alloter.h"

#define WSAEWOULDBLOCK 10035  // windows error

namespace cppnet {

ConnectSocket::ConnectSocket():
  accept_event_(nullptr) {}

ConnectSocket::~ConnectSocket() {}

bool ConnectSocket::Bind(const std::string& ip, uint16_t port) {
  if (sock_ == 0) {
    auto ret = fdan::net::TcpSocket(fdan::Address::IsIpv4(ip));
    if (ret.return_value < 0) {
      fdan::LOG_ERROR("create socket failed. errno:%d, info:%s",
        ret.err, fdan::ErrnoInfo(ret.err));
      return false;
    }
    sock_ = ret.return_value;
  }
  addr_.SetType(fdan::Address::IsIpv4(ip) ? fdan::AT_IPV4 : fdan::AT_IPV6);
  addr_.SetIp(ip);
  addr_.SetAddrPort(port);

  auto ret = fdan::net::Bind(sock_, addr_);

#ifndef __win__  // WEPOLL don't support reuse_port
  if (ret.return_value < 0 && __reuse_port) {
    fdan::LOG_FATAL("bind socket filed! error:%d, info:%s",
      ret.err, fdan::ErrnoInfo(ret.err));
    fdan::net::Close(sock_);
    return false;
  }
#endif  // __win__

  return true;
}

bool ConnectSocket::Listen() {
  auto ret = fdan::net::Listen(sock_);
#ifndef __win__  // WEPOLL don't support reuse_port
  if (ret.return_value < 0 && __reuse_port) {
    fdan::LOG_FATAL("listen socket filed! error:%d, info:%s",
      ret.err, fdan::ErrnoInfo(ret.err));
    fdan::net::Close(sock_);
    return false;
  }
#endif  // __win__

  // set the socket noblocking
  fdan::SocketNoblocking(sock_);

  Accept();

  return true;
}

void ConnectSocket::Accept() {
  if (!accept_event_) {
    accept_event_ = new Event();
    accept_event_->SetSocket(shared_from_this());
  }
  __all_socket_map[sock_] = shared_from_this();
  auto actions = GetEventActions();
  if (actions) {
    actions->AddAcceptEvent(accept_event_);
  }
}

void ConnectSocket::Close() {
  // TODO
}

void ConnectSocket::OnAccept() {
  while (true) {
    std::shared_ptr<fdan::AlloterWrap> alloter
      = std::make_shared<fdan::AlloterWrap>(fdan::MakePoolAlloterPtr());
    fdan::Address address;
    // may get more than one connections
    auto ret = fdan::net::Accept(sock_, address);
    if (ret.return_value < 0) {
      if (ret.err == EAGAIN || ret.err == WSAEWOULDBLOCK) {
        break;
      }
      fdan::LOG_ERROR("accept socket filed! errno:%d, info:%s",
        ret.err, fdan::ErrnoInfo(ret.err));
      break;
    }

    auto cppnet_base = cppnet_base_.lock();
    if (!cppnet_base) {
      return;
    }

    // set the socket noblocking
    fdan::SocketNoblocking(ret.return_value);

    // create a new socket.
    auto sock = MakeRWSocket(ret.return_value, alloter);

    sock->SetListenPort(addr_.GetAddrPort());
    sock->SetCppNetBase(cppnet_base);
    sock->SetEventActions(event_actions_);
    sock->SetAddress(std::move(address));
    sock->SetDispatcher(GetDispatcher());

    __all_socket_map[ret.return_value] = sock;

    // call accept call back function
    cppnet_base->OnAccept(sock);

    // start read
    sock->Read();
  }
}

std::shared_ptr<ConnectSocket> MakeConnectSocket() {
  return std::make_shared<ConnectSocket>();
}

}  // namespace cppnet
