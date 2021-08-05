// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)
// Copyright <caozhiyi5@gmail.com>

#include <errno.h>

#include <vector>

#include "cppnet/dispatcher.h"
#include "cppnet/cppnet_base.h"
#include "cppnet/cppnet_config.h"
#include "cppnet/socket/rw_socket.h"
#include "cppnet/event/event_interface.h"
#include "cppnet/event/action_interface.h"

#include "foundation/log/log.h"
#include "foundation/network/socket.h"
#include "foundation/alloter/pool_block.h"
#include "foundation/buffer/buffer_queue.h"
#include "foundation/alloter/pool_alloter.h"

namespace cppnet {

thread_local std::unordered_map<uint64_t,                  \
  std::shared_ptr<Socket>> RWSocket::connecting_socket_map_;

RWSocket::RWSocket():
  RWSocket(0, std::make_shared<fdan::AlloterWrap>(fdan::MakePoolAlloterPtr())) {}

RWSocket::RWSocket(std::shared_ptr<fdan::AlloterWrap> alloter):
  RWSocket(0, alloter) {}

RWSocket::RWSocket(uint64_t sock, std::shared_ptr<fdan::AlloterWrap> alloter):
  Socket(sock),
  context_(nullptr),
  timer_id_(0),
  listen_port_(0),
  shutdown_(false),
  connecting_(false),
  event_(nullptr),
  alloter_(alloter) {
  block_pool_ = alloter_->PoolNewSharePtr<fdan::BlockMemoryPool>(__mem_block_size,
    __mem_block_add_step);

  write_buffer_ = alloter_->PoolNewSharePtr<fdan::BufferQueue>(block_pool_, alloter_);
  read_buffer_ = alloter_->PoolNewSharePtr<fdan::BufferQueue>(block_pool_, alloter_);
}

RWSocket::~RWSocket() {
  write_buffer_.reset();
  read_buffer_.reset();
  if (timer_id_ > 0) {
    auto dispatcher = GetDispatcher();
    if (dispatcher) {
      dispatcher->StopTimer(timer_id_);
    }
    timer_id_ = 0;
  }
}

bool RWSocket::GetAddress(std::string& ip, uint16_t& port) {
  ip = addr_.GetIp();
  port = addr_.GetAddrPort();
  return true;
}

void RWSocket::Close() {
  Disconnect();
  if (timer_id_ > 0) {
    auto dispatcher = GetDispatcher();
    if (dispatcher) {
      dispatcher->StopTimer(timer_id_);
    }
    timer_id_ = 0;
  }
}

void RWSocket::Read() {
  if (!event_) {
    event_ = alloter_->PoolNew<Event>();
    event_->SetSocket(shared_from_this());
  }

  auto actions = GetEventActions();
  if (actions) {
    actions->AddRecvEvent(event_);
  }
}

bool RWSocket::Write(const char* src, uint32_t len) {
  if (!event_) {
    event_ = alloter_->PoolNew<Event>();
    event_->SetSocket(shared_from_this());
  }

  // can't send now
  if (write_buffer_->GetCanReadLength() > 0) {
    if (write_buffer_->GetCanReadLength() > __max_write_cache) {
      return false;
    }

    write_buffer_->Write(src, len);
    auto actions = GetEventActions();
    if (actions) {
      return actions->AddSendEvent(event_);
    }
    return false;

  } else {
    write_buffer_->Write(src, len);
    return Send();
  }
}

void RWSocket::Connect(const std::string& ip, uint16_t port) {
  if (!event_) {
    event_ = alloter_->PoolNew<Event>();
    event_->SetSocket(shared_from_this());
  }

  bool use_ipv4 = false;
  if (fdan::Address::IsIpv4(ip)) {
    use_ipv4 = true;
  }
  if (sock_ == 0) {
    auto ret = fdan::net::TcpSocket(use_ipv4);
    if (ret.return_value < 0) {
      fdan::LOG_ERROR("create socket failed. error:%d", ret.err);
      return;
    }
    sock_ = ret.return_value;
  }

  addr_.SetType(use_ipv4 ? fdan::AT_IPV4 : fdan::AT_IPV6);
  addr_.SetIp(ip);
  addr_.SetAddrPort(port);

  auto actions = GetEventActions();
  if (actions) {
    connecting_ = true;
    actions->AddConnection(event_, addr_);
    connecting_socket_map_[sock_] = shared_from_this();
  }
}

void RWSocket::Disconnect() {
  if (!event_) {
    event_ = alloter_->PoolNew<Event>();
    event_->SetSocket(shared_from_this());
  }

  auto actions = GetEventActions();
  if (actions) {
    actions->AddDisconnection(event_);
  }
}

void RWSocket::AddTimer(uint32_t interval, bool always) {
  if (timer_id_ > 0) {
    return;
  }

  auto dispatcher = GetDispatcher();
  if (dispatcher) {
    timer_id_ = dispatcher->AddTimer(shared_from_this(), interval, always);
  }
}

void RWSocket::StopTimer() {
  if (timer_id_ == 0) {
    return;
  }

  auto dispatcher = GetDispatcher();
  if (dispatcher) {
    dispatcher->StopTimer(timer_id_);
    timer_id_ = 0;
  }
}

void RWSocket::OnTimer() {
  if (connecting_) {
    connecting_socket_map_.erase(sock_);
    if (fdan::CheckConnect(GetSocket())) {
      OnConnect(CEC_SUCCESS);
    } else {
      OnConnect(CEC_CONNECT_REFUSE);
    }
    return;
  }
  auto cppnet_base = cppnet_base_.lock();
  if (!cppnet_base) {
    return;
  }
  cppnet_base->OnTimer(shared_from_this());
}

void RWSocket::OnRead(uint32_t len) {
  Recv(len);
}

void RWSocket::OnWrite(uint32_t len) {
  Send();
}

void RWSocket::OnConnect(uint16_t err) {
  connecting_socket_map_.erase(sock_);
  connecting_ = false;
  auto sock = shared_from_this();
  if (err == CEC_SUCCESS) {
    __all_socket_map[sock_] = sock;
  }

  auto cppnet_base = cppnet_base_.lock();
  if (cppnet_base) {
    cppnet_base->OnConnect(sock, err);
  }

  if (err == CEC_SUCCESS) {
    Read();
  }
}

void RWSocket::OnDisConnect(uint16_t err) {
  auto sock = shared_from_this();
  __all_socket_map.erase(sock_);

  if (!IsShutdown()) {
    auto cppnet_base = cppnet_base_.lock();
    if (cppnet_base) {
      cppnet_base->OnDisConnect(sock, err);
    }
  }
  SetShutdown();

  // peer disconnect or connection break.
  if (event_ && err != CEC_SUCCESS) {
    auto actions = GetEventActions();
    if (actions) {
      actions->DelEvent(event_);
    }
    fdan::net::Close(sock_);
  }
}

bool RWSocket::Recv(uint32_t len) {
  auto cppnet_base = cppnet_base_.lock();
  if (!cppnet_base) {
    return false;
  }
  if (len == 0) {
    len = __linux_read_buff_expand_len;
  }

  uint32_t off_set = 0;
  // read all data.
  uint32_t expand_buff_len = len;
  bool need_expend = false;
  while (true) {
    uint32_t expand = 0;
    if (need_expend) {
      expand = expand_buff_len;
      if (expand_buff_len < __linux_read_buff_expand_max) {
        expand_buff_len *= 2;
      }
    }

    std::vector<fdan::net::Iovec> io_vec;
    uint32_t buff_len = read_buffer_->GetFreeMemoryBlock(io_vec, expand);
    auto ret = fdan::net::Readv(sock_, &*io_vec.begin(), io_vec.size());
    if (ret.return_value < 0) {
      if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR) {
        read_buffer_->MoveWritePt(ret.return_value);
        off_set += ret.return_value;
        break;

      } else {
        OnDisConnect(CEC_CONNECT_BREAK);
        return false;
      }

    } else if (ret.return_value == 0) {
      OnDisConnect(CEC_CLOSED);
      return false;

    } else {
      read_buffer_->MoveWritePt(ret.return_value);
      off_set += ret.return_value;
      // read all
      if ((uint32_t)ret.return_value < buff_len) {
        break;
      }
      need_expend = true;
    }
  }

  cppnet_base->OnRead(shared_from_this(), read_buffer_, off_set);
  return true;
}

bool RWSocket::Send() {
  auto cppnet_base = cppnet_base_.lock();
  if (!cppnet_base) {
    return false;
  }

  uint32_t off_set = 0;
  while (write_buffer_ && write_buffer_->GetCanReadLength() > 0) {
    std::vector<fdan::net::Iovec> io_vec;
    write_buffer_->GetUseMemoryBlock(io_vec, __linux_write_buff_get);
    auto ret = fdan::net::Writev(sock_, &*io_vec.begin(), io_vec.size());
    if (ret.return_value >= 0) {
      write_buffer_->MoveReadPt(ret.return_value);
      off_set += ret.return_value;

    } else {
      if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR) {
        // can't send complete
        auto actions = GetEventActions();
        if (actions) {
          return actions->AddSendEvent(event_);
        }
        return false;

      } else if (errno == EBADMSG) {
        OnDisConnect(CEC_CONNECT_BREAK);
        return false;

      } else {
        OnDisConnect(CEC_CLOSED);
        return false;
      }
    }
  }
  cppnet_base->OnWrite(shared_from_this(), off_set);
  return true;
}

std::shared_ptr<RWSocket> MakeRWSocket() {
  return std::make_shared<RWSocket>();
}

std::shared_ptr<RWSocket> MakeRWSocket(std::shared_ptr<fdan::AlloterWrap> alloter) {
  return std::make_shared<RWSocket>(alloter);
}

std::shared_ptr<RWSocket> MakeRWSocket(uint64_t sock,
  std::shared_ptr<fdan::AlloterWrap> alloter) {
  return std::make_shared<RWSocket>(sock, alloter);
}

}  // namespace cppnet
