// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)
// Copyright <caozhiyi5@gmail.com>

#ifndef CPPNET_DISPATCHER_H_
#define CPPNET_DISPATCHER_H_

#include <mutex>
#include <memory>
#include <thread>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

#include "include/cppnet_type.h"
#include "foundation/timer/timer.h"
#include "foundation/thread/thread_with_queue.h"

namespace cppnet {

typedef std::function<void()> Task;

class RWSocket;
class TimerEvent;
class CppNetBase;
class EventActions;

class Dispatcher:
  public fdan::Thread,
  public std::enable_shared_from_this<Dispatcher> {
 public:
  Dispatcher(std::shared_ptr<CppNetBase> base,
    uint32_t thread_num, uint32_t base_id);
  explicit Dispatcher(std::shared_ptr<CppNetBase> base,
    uint32_t base_id = 0);
  ~Dispatcher();

  void Run();

  void Stop();

  void Listen(uint64_t sock, const std::string& ip, uint16_t port);

  void Connect(const std::string& ip, uint16_t port);

  void PostTask(const Task& task);

  uint32_t AddTimer(const user_timer_call_back& cb, void* param,
    uint32_t interval, bool always = false);
  uint32_t AddTimer(std::shared_ptr<RWSocket> sock, uint32_t interval,
    bool always = false);
  void StopTimer(uint32_t timer_id);

  std::thread::id GetThreadID() { return local_thread_id_; }

 private:
  void DoTask();
  uint32_t MakeTimerID();

 private:
  uint64_t cur_utc_time_;

  std::mutex timer_id_mutex_;
  uint32_t   timer_id_creater_;

  std::mutex task_list_mutex_;
  std::vector<Task> task_list_;

  std::thread::id local_thread_id_;
  std::shared_ptr<fdan::Timer> timer_;
  std::shared_ptr<EventActions> event_actions_;

  std::weak_ptr<CppNetBase> cppnet_base_;

  static thread_local std::unordered_map<uint64_t,   \
    std::shared_ptr<TimerEvent>> all_timer_event_map_;
};

}  // namespace cppnet

#endif  // CPPNET_DISPATCHER_H_
