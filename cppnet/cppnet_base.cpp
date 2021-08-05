// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)
// Copyright <caozhiyi5@gmail.com>

#ifndef __win__
#include <signal.h>
#endif

#include "cppnet/dispatcher.h"
#include "cppnet/cppnet_base.h"
#include "include/cppnet_type.h"
#include "cppnet/cppnet_config.h"
#include "include/cppnet_buffer.h"
#include "cppnet/socket/rw_socket.h"

#include "foundation/log/log.h"
#include "foundation/os/os_info.h"
#include "foundation/util/random.h"
#include "foundation/network/socket.h"
#include "foundation/network/net_handle.h"
#include "foundation/buffer/buffer_queue.h"

namespace cppnet {

union TimerId {
  struct {
    uint32_t sub_timer_id;
    uint32_t dispatcher_index;
  } detail_info;
  uint64_t timer_id;
};


CppNetBase::CppNetBase() {}

CppNetBase::~CppNetBase() {}

void CppNetBase::Init(uint32_t thread_num) {
  uint32_t cpus = fdan::GetCpuNum();
  if (thread_num == 0 || thread_num >= cpus * 2) {
    thread_num = cpus;
  }
  random_ = std::unique_ptr<fdan::RangeRandom>(
    new fdan::RangeRandom(0, thread_num - 1));

#ifndef __win__
  // Disable  SIGPIPE signal
  sigset_t set;
  sigprocmask(SIG_SETMASK, NULL, &set);
  sigaddset(&set, SIGPIPE);
  sigprocmask(SIG_SETMASK, &set, NULL);
#endif  // __win__

  for (uint32_t i = 0; i < thread_num; i++) {
    auto dispatcher = std::make_shared<Dispatcher>(shared_from_this());
    dispatchers_.push_back(dispatcher);
  }
}

void CppNetBase::Dealloc() {
  for (size_t i = 0; i < dispatchers_.size(); i++) {
    dispatchers_[i]->Stop();
  }
}

void CppNetBase::Join() {
  for (size_t i = 0; i < dispatchers_.size(); i++) {
    dispatchers_[i]->Join();
  }
}

uint64_t CppNetBase::AddTimer(uint32_t interval,
  const user_timer_call_back& cb, void* param, bool always) {
  uint32_t index = random_->Random();
  uint32_t id = dispatchers_[index]->AddTimer(cb, param, interval, always);
  TimerId tid;
  tid.detail_info.dispatcher_index = index;
  tid.detail_info.sub_timer_id = id;
  return tid.timer_id;
}

void CppNetBase::RemoveTimer(uint64_t timer_id) {
  TimerId tid;
  tid.timer_id = timer_id;
  dispatchers_[tid.detail_info.dispatcher_index]->StopTimer(
    tid.detail_info.sub_timer_id);
}

bool CppNetBase::ListenAndAccept(const std::string& ip, uint16_t port) {
#ifdef __win__   // WEPOLL don't support reuse_port
  auto ret = fdan::net::TcpSocket(Address::IsIpv4(ip));
  if (ret.return_value < 0) {
    fdan::LOG_ERROR("create socket failed. err:%d", ret.err);
    return false;
  }
  for (size_t i = 0; i < dispatchers_.size(); i++) {
    dispatchers_[i]->Listen(ret._return_value, ip, port);
  }
#else  // __win__
  if (__reuse_port) {
    for (size_t i = 0; i < dispatchers_.size(); i++) {
      auto ret = fdan::net::TcpSocket(fdan::Address::IsIpv4(ip));
      if (ret.return_value < 0) {
        fdan::LOG_ERROR("create socket failed. err:%d", ret.err);
        return false;
      }
      fdan::ReusePort(ret.return_value);
      dispatchers_[i]->Listen(ret.return_value, ip, port);
    }

  } else {
    auto ret = fdan::net::TcpSocket(fdan::Address::IsIpv4(ip));
    if (ret.return_value < 0) {
      fdan::LOG_ERROR("create socket failed. err:%d", ret.err);
      return false;
    }
    for (size_t i = 0; i < dispatchers_.size(); i++) {
      dispatchers_[i]->Listen(ret.return_value, ip, port);
    }
  }
#endif
  return true;
}

bool CppNetBase::Connection(const std::string& ip, uint16_t port) {
  uint32_t index = random_->Random();
  dispatchers_[index]->Connect(ip, port);
  return true;
}

void CppNetBase::OnTimer(std::shared_ptr<RWSocket> sock) {
  if (timer_cb_) {
    timer_cb_(sock);
  }
}

void CppNetBase::OnAccept(std::shared_ptr<RWSocket> sock) {
  if (accept_cb_) {
    accept_cb_(sock, CEC_SUCCESS);
  }
}

void CppNetBase::OnRead(std::shared_ptr<RWSocket> sock,
  std::shared_ptr<fdan::Buffer> buffer, uint32_t len) {
  if (read_cb_) {
    BufferPtr buff = std::make_shared<Buffer>(buffer);
    read_cb_(sock, buff, len);
  }
}

void CppNetBase::OnWrite(std::shared_ptr<RWSocket> sock, uint32_t len) {
  if (write_cb_) {
    write_cb_(sock, len);
  }
}

void CppNetBase::OnConnect(std::shared_ptr<RWSocket> sock, uint16_t err) {
  if (connect_cb_) {
    connect_cb_(sock, err);
  }
}

void CppNetBase::OnDisConnect(std::shared_ptr<RWSocket> sock, uint16_t err) {
  if (disconnect_cb_) {
    disconnect_cb_(sock, err);
  }
}

}  // namespace cppnet
