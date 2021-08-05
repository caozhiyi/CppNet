// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)
// Copyright <caozhiyi5@gmail.com>

#ifndef CPPNET_SOCKET_RW_SOCKET_H_
#define CPPNET_SOCKET_RW_SOCKET_H_

#include <string>
#include <atomic>
#include <memory>
#include <unordered_map>

#include "include/cppnet_socket.h"
#include "cppnet/socket/socket_interface.h"
#include "foundation/buffer/buffer_queue.h"
#include "foundation/alloter/alloter_interface.h"

namespace cppnet {

class Event;
class BlockMemoryPool;
class RWSocket:
  public Socket,
  public CNSocket,
  public std::enable_shared_from_this<RWSocket> {
 public:
  RWSocket();
  explicit RWSocket(std::shared_ptr<fdan::AlloterWrap> alloter);
  RWSocket(uint64_t sock, std::shared_ptr<fdan::AlloterWrap> alloter);
  virtual ~RWSocket();

  virtual uint64_t GetSocket() { return sock_; }
  virtual void SetListenPort(uint16_t port) { listen_port_ = port; }
  virtual uint16_t GetListenPort() { return listen_port_; }
  virtual bool GetAddress(std::string& ip, uint16_t& port);

  virtual void Close();

  virtual void Read();
  virtual bool Write(const char* src, uint32_t len);
  virtual void Connect(const std::string& ip, uint16_t port);
  virtual void Disconnect();

  virtual void AddTimer(uint32_t interval, bool always = false);
  virtual void StopTimer();

  virtual void OnTimer();
  virtual void OnRead(uint32_t len = 0);
  virtual void OnWrite(uint32_t len = 0);
  virtual void OnConnect(uint16_t err);
  virtual void OnDisConnect(uint16_t err);

  virtual void SetContext(void* context) { context_ = context; }
  virtual void* GetContext() { return context_; }

  virtual void SetShutdown() { shutdown_ = true; }
  virtual bool IsShutdown() { return shutdown_; }

  std::shared_ptr<fdan::AlloterWrap> GetAlloter() { return alloter_; }

 private:
  bool Recv(uint32_t len);
  bool Send();

 protected:
  void*  context_;
  uint32_t timer_id_;
  uint16_t listen_port_;
  std::atomic_bool shutdown_;
  std::atomic_bool connecting_;
  Event*           event_;

  std::shared_ptr<fdan::BufferQueue>   write_buffer_;
  std::shared_ptr<fdan::BufferQueue>   read_buffer_;

  std::shared_ptr<fdan::AlloterWrap>     alloter_;
  std::shared_ptr<fdan::BlockMemoryPool> block_pool_;

  static thread_local std::unordered_map<uint64_t,    \
    std::shared_ptr<Socket>> connecting_socket_map_;
};

std::shared_ptr<RWSocket> MakeRWSocket();
std::shared_ptr<RWSocket> MakeRWSocket(               \
  std::shared_ptr<fdan::AlloterWrap> alloter);
std::shared_ptr<RWSocket> MakeRWSocket(uint64_t sock, \
  std::shared_ptr<fdan::AlloterWrap> alloter);

}  // namespace cppnet

#endif  // CPPNET_SOCKET_RW_SOCKET_H_
