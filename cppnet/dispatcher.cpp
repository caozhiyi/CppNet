// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)
// Copyright <caozhiyi5@gmail.com>

#include <map>

#include "cppnet/dispatcher.h"
#include "cppnet/cppnet_base.h"
#include "cppnet/socket/rw_socket.h"
#include "cppnet/event/timer_event.h"
#include "cppnet/socket/connect_socket.h"
#include "cppnet/event/action_interface.h"

#include "foundation/util/time.h"
#include "foundation/alloter/pool_alloter.h"

namespace cppnet {

thread_local std::unordered_map<uint64_t,                      \
  std::shared_ptr<TimerEvent>> Dispatcher::all_timer_event_map_;

Dispatcher::Dispatcher(std::shared_ptr<CppNetBase> base,
  uint32_t thread_num, uint32_t base_id):
  cur_utc_time_(0),
  timer_id_creater_(0),
  cppnet_base_(base) {
  timer_ = fdan::MakeTimer1Min();

  event_actions_ = MakeEventActions();
  event_actions_->Init();

  // start thread
  Start();
}

Dispatcher::Dispatcher(std::shared_ptr<CppNetBase> base, uint32_t base_id):
  cur_utc_time_(0),
  timer_id_creater_(0),
  cppnet_base_(base) {
  timer_ = fdan::MakeTimer1Min();

  event_actions_ = MakeEventActions();
  event_actions_->Init();

  // start thread
  Start();
}

Dispatcher::~Dispatcher() {
  if (std::this_thread::get_id() != local_thread_id_) {
    Stop();
    Join();
  }
}

void Dispatcher::Run() {
  local_thread_id_ = std::this_thread::get_id();
  cur_utc_time_ = fdan::UTCTimeMsec();
  int32_t wait_time = 0;
  uint64_t cur_time = 0;

  while (!_stop) {
    cur_time = fdan::UTCTimeMsec();
    timer_->TimerRun(uint32_t(cur_time - cur_utc_time_));
    cur_utc_time_ = cur_time;

    if (_stop) {
      break;
    }

    wait_time = timer_->MinTime();

    event_actions_->ProcessEvent(wait_time);

    DoTask();
  }
}

void Dispatcher::Stop() {
  _stop = true;
  event_actions_->Wakeup();
}

void Dispatcher::Listen(uint64_t sock, const std::string& ip, uint16_t port) {
  auto task = [sock, ip, port, this]() {
    auto connect_sock = MakeConnectSocket();
    connect_sock->SetEventActions(event_actions_);
    connect_sock->SetCppNetBase(cppnet_base_.lock());
    connect_sock->SetSocket(sock);
    connect_sock->SetDispatcher(shared_from_this());

    connect_sock->Bind(ip, port);
    connect_sock->Listen();
  };

  if (std::this_thread::get_id() == local_thread_id_) {
    task();

  } else {
    PostTask(task);
  }
}

void Dispatcher::Connect(const std::string& ip, uint16_t port) {
  auto task = [ip, port, this]() {
    auto sock = MakeRWSocket();
    sock->SetDispatcher(shared_from_this());
    sock->SetEventActions(event_actions_);
    sock->SetCppNetBase(cppnet_base_.lock());
    sock->Connect(ip, port);
  };

  if (std::this_thread::get_id() == local_thread_id_) {
    task();

  } else {
    PostTask(task);
  }
}

void Dispatcher::PostTask(const Task& task) {
  {
    std::unique_lock<std::mutex> lock(task_list_mutex_);
    task_list_.push_back(task);
  }
  event_actions_->Wakeup();
}

uint32_t Dispatcher::AddTimer(const user_timer_call_back& cb,
  void* param, uint32_t interval, bool always) {
  std::shared_ptr<TimerEvent> event = std::make_shared<TimerEvent>();
  event->AddType(ET_USER_TIMER);
  event->SetTimerCallBack(cb, param);

  uint32_t timer_id = MakeTimerID();

  if (std::this_thread::get_id() == local_thread_id_) {
    timer_->AddTimer(event, interval, always);
    all_timer_event_map_[timer_id] = event;
    event_actions_->Wakeup();

  } else {
    auto task = [event, timer_id, interval, always, this]() {
      timer_->AddTimer(event, interval, always);
      all_timer_event_map_[timer_id] = event;
    };
    PostTask(task);
  }
  return timer_id;
}

uint32_t Dispatcher::AddTimer(std::shared_ptr<RWSocket> sock,
  uint32_t interval, bool always) {
  std::shared_ptr<TimerEvent> event = std::make_shared<TimerEvent>();
  event->AddType(ET_TIMER);
  event->SetSocket(sock);

  uint32_t timer_id = MakeTimerID();

  if (std::this_thread::get_id() == local_thread_id_) {
    timer_->AddTimer(event, interval, always);
    all_timer_event_map_[timer_id] = event;
    event_actions_->Wakeup();

  } else {
    auto task = [event, timer_id, interval, always, this]() {
      timer_->AddTimer(event, interval, always);
      all_timer_event_map_[timer_id] = event;
    };
    PostTask(task);
  }
  return timer_id;
}

void Dispatcher::StopTimer(uint32_t timer_id) {
  if (std::this_thread::get_id() == local_thread_id_) {
    auto iter = all_timer_event_map_.find(timer_id);
    if (iter == all_timer_event_map_.end()) {
      return;
    }

    timer_->RmTimer(iter->second);
    all_timer_event_map_.erase(iter);

  } else {
    auto task = [timer_id, this]() {
      auto iter = all_timer_event_map_.find(timer_id);
      if (iter == all_timer_event_map_.end()) {
        return;
      }

      timer_->RmTimer(iter->second);
      all_timer_event_map_.erase(iter);
    };
    PostTask(task);
  }
}

void Dispatcher::DoTask() {
  std::vector<Task> func_vec;
  {
    std::unique_lock<std::mutex> lock(task_list_mutex_);
    func_vec.swap(task_list_);
  }

  for (std::size_t i = 0; i < func_vec.size(); ++i) {
    func_vec[i]();
  }
}

uint32_t Dispatcher::MakeTimerID() {
  std::unique_lock<std::mutex> lock(timer_id_mutex_);
  return ++timer_id_creater_;
}

}  // namespace cppnet
