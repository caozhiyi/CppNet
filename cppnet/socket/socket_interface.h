// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)
// Copyright <caozhiyi5@gmail.com>

#ifndef CPPNET_SOCKET_SOCKET_INTERFACE_H_
#define CPPNET_SOCKET_SOCKET_INTERFACE_H_

#include <memory>
#include <cstdint>
#include <unordered_map>

#include "foundation/network/address.h"

namespace cppnet {

class CppNetBase;
class Dispatcher;
class EventActions;
class Socket {
 public:
  Socket(): sock_(0) {}
  explicit Socket(uint64_t sock): sock_(sock) {}
  virtual ~Socket() {}

  void SetSocket(const uint64_t& sock) { sock_ = sock; }
  uint64_t GetSocket() { return sock_; }

  void SetAddress(const fdan::Address& addr) { addr_ = addr; }
  const fdan::Address& GetAddress() const { return addr_; }

  void SetCppNetBase(std::shared_ptr<CppNetBase> base)
    { cppnet_base_ = base; }
  const std::shared_ptr<CppNetBase> GetCppNetBase() const
    { return cppnet_base_.lock(); }

  void SetEventActions(std::weak_ptr<EventActions> actions)
    { event_actions_ = actions; }
  const std::shared_ptr<EventActions> GetEventActions() const
    { return event_actions_.lock(); }

  void SetDispatcher(std::shared_ptr<Dispatcher> dis) { dispatcher_ = dis; }
  std::shared_ptr<Dispatcher> GetDispatcher() { return dispatcher_.lock(); }

 protected:
  uint64_t      sock_;
  fdan::Address addr_;

  std::weak_ptr<CppNetBase>   cppnet_base_;
  std::weak_ptr<EventActions> event_actions_;
  std::weak_ptr<Dispatcher>   dispatcher_;

  static thread_local std::unordered_map<uint64_t,  \
    std::shared_ptr<Socket>> __all_socket_map;
};

}  // namespace cppnet

#endif  // CPPNET_SOCKET_SOCKET_INTERFACE_H_
