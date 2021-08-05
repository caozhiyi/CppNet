// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)
// Copyright <caozhiyi5@gmail.com>

#ifndef CPPNET_SOCKET_CONNECT_SOCKET_H_
#define CPPNET_SOCKET_CONNECT_SOCKET_H_

#include <memory>
#include <string>

#include "cppnet/socket/socket_interface.h"

namespace cppnet {

class Event;
class ConnectSocket:
  public Socket,
  public std::enable_shared_from_this<ConnectSocket> {
 public:
  ConnectSocket();
  virtual ~ConnectSocket();

  virtual bool Bind(const std::string& ip, uint16_t port);
  virtual bool Listen();
  virtual void Accept();
  virtual void Close();

  virtual void OnAccept();

 private:
  Event*  accept_event_;
};

std::shared_ptr<ConnectSocket> MakeConnectSocket();

}  // namespace cppnet

#endif  // CPPNET_SOCKET_CONNECT_SOCKET_H_
