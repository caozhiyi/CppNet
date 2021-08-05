// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)
// Copyright <caozhiyi5@gmail.com>

#include <unistd.h>   // for close

#include <memory>
#include <thread>

#include "include/cppnet_type.h"
#include "cppnet/socket/rw_socket.h"
#include "cppnet/socket/connect_socket.h"
#include "cppnet/event/event_interface.h"
#include "cppnet/event/kqueue/kqueue_action.h"

#include "foundation/log/log.h"
#include "foundation/util/time.h"
#include "foundation/os/convert.h"
#include "foundation/util/os_return.h"
#include "foundation/network/socket.h"
#include "foundation/network/address.h"
#include "foundation/network/net_handle.h"
#include "foundation/timer/timer_interface.h"

namespace cppnet {

std::shared_ptr<EventActions> MakeEventActions() {
  return std::make_shared<KqueueEventActions>();
}

KqueueEventActions::KqueueEventActions():
  kqueue_handler_(-1) {
  active_list_.resize(1024);
  kqueue_timeout_.tv_nsec = 0;
  kqueue_timeout_.tv_sec = 0;
}

KqueueEventActions::~KqueueEventActions() {
  if (kqueue_handler_ > 0) {
    fdan::net::Close(kqueue_handler_);
  }
  if (pipe_[0] > 0) {
    fdan::net::Close(pipe_[0]);
  }
  if (pipe_[0] > 0) {
    fdan::net::Close(pipe_[0]);
  }
}

bool KqueueEventActions::Init(uint32_t thread_num) {
  kqueue_handler_ = kqueue();
  if (kqueue_handler_ < 0) {
    fdan::fdan::LOG_ERROR("create kqueue failed. errno:%d", errno);
    return false;
  }

  if (reinterpret_cast<int*>(pipe_) == -1) {
    fdan::LOG_FATAL("pipe init failed! error : %d", errno);
    return false;
  }

  fdan::SocketNoblocking(pipe_[0]);
  fdan::SocketNoblocking(pipe_[1]);

  struct kevent ev;
  EV_SET(&ev, pipe_[0], EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);

  change_list_.push_back(ev);
  return true;
}

bool KqueueEventActions::Dealloc() {
  Wakeup();
  return true;
}

bool KqueueEventActions::AddSendEvent(Event* event) {
  if (event->GetType() & ET_WRITE) {
    return false;
  }
  event->AddType(ET_WRITE);

  auto sock = event->GetSocket();
  if (sock) {
    struct kevent ev;
    EV_SET(&ev, sock->GetSocket(), EVFILT_WRITE,       \
      EV_ADD | EV_ENABLE | EV_CLEAR, 0, 0, (void*)event);

    change_list_.push_back(ev);
    return true;
  }
  fdan::fdan::LOG_WARN("socket is already distroyed! event %s", "AddSendEvent");
  return false;
}

bool KqueueEventActions::AddRecvEvent(Event* event) {
  if (event->GetType() & ET_READ) {
    return false;
  }
  event->AddType(ET_READ);

  auto sock = event->GetSocket();
  if (sock) {
    struct kevent ev;
    EV_SET(&ev, sock->GetSocket(), EVFILT_READ,       \
      EV_ADD | EV_ENABLE, 0, 0, (void*)event);
    change_list_.push_back(ev);
    return true;
  }
  fdan::LOG_WARN("socket is already distroyed! event %s", "AddRecvEvent");
  return false;
}

bool KqueueEventActions::AddAcceptEvent(Event* event) {
  if (event->GetType() & ET_ACCEPT) {
    return false;
  }
  event->AddType(ET_ACCEPT);

  auto sock = event->GetSocket();
  if (sock) {
    struct kevent ev;
    EV_SET(&ev, sock->GetSocket(), EVFILT_READ,       \
      EV_ADD | EV_ENABLE, 0, 0, (void*)event);

    change_list_.push_back(ev);
    return true;
  }
  fdan::LOG_WARN("socket is already distroyed! event %s", "AddAcceptEvent");
  return false;
}

bool KqueueEventActions::AddConnection(Event* event, Address& address) {
  if (event->GetType() & ET_CONNECT) {
    return false;
  }
  event->AddType(ET_CONNECT);

  auto sock = event->GetSocket();
  if (sock) {
    SocketNoblocking(sock->GetSocket());

    auto ret = fdan::net::Connect(sock->GetSocket(), address);

    auto rw_sock = std::dynamic_pointer_cast<RWSocket>(sock);
    if (ret._return_value == 0) {
      rw_sock->OnConnect(CEC_SUCCESS);
      return true;

    } else if (ret._errno == EINPROGRESS) {
      if (CheckConnect(rw_sock->GetSocket())) {
        rw_sock->OnConnect(CEC_SUCCESS);
        return true;
      }
    }
    rw_sock->OnConnect(CEC_CONNECT_REFUSE);
    fdan::LOG_ERROR("connect to peer failed! errno:%d, info:%s",
      ret._errno, ErrnoInfo(ret._errno));
    return false;
  }

  fdan::LOG_WARN("socket is already destroyed!, event:%s", "AddConnection");
  return false;
}

bool KqueueEventActions::AddDisconnection(Event* event) {
  if (event->GetType() & ET_DISCONNECT) {
    return false;
  }
  event->AddType(ET_DISCONNECT);

  auto sock = event->GetSocket();
  if (!sock) {
    return false;
  }

  std::shared_ptr<RWSocket> socket = std::dynamic_pointer_cast<RWSocket>(sock);
  fdan::net::Close(socket->GetSocket());
  socket->OnDisConnect(CEC_SUCCESS);
  return true;
}

bool KqueueEventActions::DelEvent(Event* event) {
  auto sock = event->GetSocket();
  if (!sock) {
    return false;
  }

  struct kevent read_ev;
  EV_SET(&read_ev, sock->GetSocket(), EVFILT_READ,       \
    EV_DELETE | EV_DISABLE, 0, 0, NULL);
  change_list_.push_back(read_ev);

  struct kevent write_ev;
  EV_SET(&write_ev, sock->GetSocket(), EVFILT_WRITE,     \
    EV_DELETE | EV_DISABLE, 0, 0, NULL);
  change_list_.push_back(write_ev);

  event->ClearType();
  return true;
}

void KqueueEventActions::ProcessEvent(int32_t wait_ms) {
  int16_t ret = 0;
  if (wait_ms > 0) {
    kqueue_timeout_.tv_sec = (uint64_t)wait_ms / 1000;
    kqueue_timeout_.tv_nsec = ((uint64_t)wait_ms -
      (kqueue_timeout_.tv_sec * 1000)) * 1000000;

    ret = kevent(kqueue_handler_, &*change_list_.begin(), (int)change_list_.size(),
      &*active_list_.begin(), (int)active_list_.size(), &kqueue_timeout_);
  } else {
    ret = kevent(kqueue_handler_, &*change_list_.begin(), (int)change_list_.size(),
      &*active_list_.begin(), (int)active_list_.size(), nullptr);
  }

  change_list_.clear();
  if (ret < 0) {
    fdan::LOG_ERROR("kevent faild! error:%d, info:%s",
      errno, ErrnoInfo(errno));

  } else {
    fdan::LOG_DEBUG("kevent get events! num:%d, TheadId:%lld",
      ret, std::this_thread::get_id());

    OnEvent(active_list_, ret);
  }
}

void KqueueEventActions::Wakeup() {
  write(pipe_[1], "1", 1);
}

void KqueueEventActions::OnEvent(std::vector<struct kevent>& event_vec,
  int16_t num) {
  std::shared_ptr<Socket> sock;
  Event* event;

  for (int i = 0; i < num; i++) {
    if (event_vec[i].ident == pipe_[0]) {
      LOG_INFO("weak up the io thread, index : %d", i);
      char buf[4];
      read(pipe_[0], buf, 1);
      continue;
    }

    if (event_vec[i].flags & EV_DELETE) {
      continue;
    }

    event = reinterpret_cast<Event*>(event_vec[i].udata);
    sock = event->GetSocket();
    if (!sock) {
      fdan::LOG_WARN("kqueue weak up but socket already destroy, index : %d", i);
      continue;
    }

    // accept event
    if (event->GetType() & ET_ACCEPT) {
      std::shared_ptr<ConnectSocket> connect_sock =
        std::dynamic_pointer_cast<ConnectSocket>(sock);
      connect_sock->OnAccept();

    } else {
      // write event
      if (event_vec[i].flags & EV_CLEAR) {
        std::shared_ptr<RWSocket> rw_sock =
          std::dynamic_pointer_cast<RWSocket>(sock);
        event->RemoveType(ET_WRITE);
        rw_sock->OnWrite(event_vec[i].data);
      // read event
      } else {
        std::shared_ptr<RWSocket> rw_sock =
          std::dynamic_pointer_cast<RWSocket>(sock);
        rw_sock->OnRead(event_vec[i].data);
      }
    }
  }
}

}  // namespace cppnet
