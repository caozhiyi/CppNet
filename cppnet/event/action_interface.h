// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)
// Copyright <caozhiyi5@gmail.com>

#ifndef CPPNET_EVENT_ACTION_INTERFACE_H_
#define CPPNET_EVENT_ACTION_INTERFACE_H_

#include <vector>
#include <string>
#include <memory>
#include <cstdint>
#include <functional>
#include <unordered_set>

#include "foundation/network/address.h"

namespace cppnet {

class Timer;
class Event;
class TimeSolt;

// net IO event interface
class EventActions {
 public:
  EventActions() {}
  virtual ~EventActions() {}

  virtual bool Init(uint32_t thread_num = 0) = 0;
  virtual bool Dealloc() = 0;

  // net IO event
  virtual bool AddSendEvent(Event* event) = 0;
  virtual bool AddRecvEvent(Event* event) = 0;
  virtual bool AddAcceptEvent(Event* event) = 0;
  virtual bool AddConnection(Event* event, const fdan::Address& addr) = 0;
  virtual bool AddDisconnection(Event* event) = 0;

  virtual bool DelEvent(Event* event) = 0;
  // IO thread process
  virtual void ProcessEvent(int32_t wait_ms) = 0;
  // weak up net IO thread
  virtual void Wakeup() = 0;
};

std::shared_ptr<EventActions> MakeEventActions();

}  // namespace cppnet

#endif  // CPPNET_EVENT_ACTION_INTERFACE_H_
