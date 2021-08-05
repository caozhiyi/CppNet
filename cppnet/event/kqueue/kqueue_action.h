// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)
// Copyright <caozhiyi5@gmail.com>

#ifndef CPPNET_EVENT_KQUEUE_KQUEUE_ACTION_H_
#define CPPNET_EVENT_KQUEUE_KQUEUE_ACTION_H_

#include <sys/event.h>  // for kqueue

#include <mutex>
#include <vector>
#include <cstdint>
#include "cppnet/event/action_interface.h"

namespace cppnet {

// kqueue event interface
class KqueueEventActions:
  public EventActions {
 public:
  KqueueEventActions();
  virtual ~KqueueEventActions();

  virtual bool Init(uint32_t thread_num = 0);
  virtual bool Dealloc();
  // net io event
  virtual bool AddSendEvent(Event* event);
  virtual bool AddRecvEvent(Event* event);
  virtual bool AddAcceptEvent(Event* event);
  virtual bool AddConnection(Event* event, const Address& address);
  virtual bool AddDisconnection(Event* event);

  virtual bool DelEvent(Event* event);
  // io thread process
  virtual void ProcessEvent(int32_t wait_ms);
  // weak up net io thread
  virtual void Wakeup();

 private:
  void OnEvent(const std::vector<struct kevent>& event_vec, int16_t num);

 protected:
  std::mutex mutex_;
  int32_t    kqueue_handler_;
  uint32_t   pipe_[2];
  timespec   kqueue_timeout_;
  std::vector<struct kevent> change_list_;
  std::vector<struct kevent> active_list_;
};

}  // namespace cppnet

#endif  // CPPNET_EVENT_KQUEUE_KQUEUE_ACTION_H_
