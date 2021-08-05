// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)
// Copyright <caozhiyi5@gmail.com>

#ifndef CPPNET_CPPNET_BASE_H_
#define CPPNET_CPPNET_BASE_H_

#include <vector>
#include <memory>
#include <string>
#include "include/cppnet_type.h"
#include "foundation/util/random.h"
#include "foundation/buffer/buffer_interface.h"

namespace cppnet {

class RWSocket;
class Dispatcher;

class CppNetBase:
  public std::enable_shared_from_this<CppNetBase> {
 public:
  CppNetBase();
  ~CppNetBase();
  // common
  void Init(uint32_t thread_num);
  void Dealloc();
  void Join();

  // set call back
  void SetReadCallback(const read_call_back& cb)
    { read_cb_ = cb; }
  void SetWriteCallback(const write_call_back& cb)
    { write_cb_ = cb; }
  void SetDisconnectionCallback(const connect_call_back& cb)
    { disconnect_cb_ = cb; }
  void SetTimerCallback(const timer_call_back& cb)
    { timer_cb_ = cb; }

  // about timer
  uint64_t AddTimer(uint32_t interval, const user_timer_call_back& cb,
    void* param = nullptr, bool always = false);
  void RemoveTimer(uint64_t timer_id);

  // server
  void SetAcceptCallback(const connect_call_back& cb)
    { accept_cb_ = cb; }
  bool ListenAndAccept(const std::string& ip, uint16_t port);

  // client
  void SetConnectionCallback(const connect_call_back& cb)
    { connect_cb_ = cb; }
  bool Connection(const std::string& ip, uint16_t port);

  // call back
  void OnTimer(std::shared_ptr<RWSocket> sock);
  void OnAccept(std::shared_ptr<RWSocket> sock);
  void OnRead(std::shared_ptr<RWSocket> sock,
    std::shared_ptr<fdan::Buffer> buffer, uint32_t len);
  void OnWrite(std::shared_ptr<RWSocket> sock, uint32_t len);
  void OnConnect(std::shared_ptr<RWSocket> sock, uint16_t err);
  void OnDisConnect(std::shared_ptr<RWSocket> sock, uint16_t err);

 private:
  timer_call_back  timer_cb_;
  read_call_back   read_cb_;
  write_call_back  write_cb_;
  connect_call_back  connect_cb_;
  connect_call_back  disconnect_cb_;
  connect_call_back  accept_cb_;

  std::unique_ptr<fdan::RangeRandom> random_;
  std::vector<std::shared_ptr<Dispatcher>> dispatchers_;
};

}  // namespace cppnet

#endif  // CPPNET_CPPNET_BASE_H_
