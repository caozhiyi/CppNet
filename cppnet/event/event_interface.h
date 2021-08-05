// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)
// Copyright <caozhiyi5@gmail.com>

#ifndef CPPNET_EVENT_EVENT_INTERFACE_H_
#define CPPNET_EVENT_EVENT_INTERFACE_H_

#include <memory>
#include <atomic>

namespace cppnet {

enum EventType {
  ET_READ         = 0x001,    // read event
  ET_WRITE        = 0x002,    // write event
  ET_ACCEPT       = 0x004,    // accept event
  ET_TIMER        = 0x008,    // timer event
  ET_USER_TIMER   = 0x010,    // timer event
  ET_CONNECT      = 0x020,    // connect event
  ET_DISCONNECT   = 0x040,    // disconnect event

  ET_INACTIONS    = 0x080,    // set to actions
};

const char* TypeString(EventType type);

class Socket;
class BufferQueue;
class Event {
 public:
  Event(): data_(nullptr), event_type_(0) {}
  virtual ~Event() {}

  void SetData(void* data) { data_ = data; }
  void* GetData() { return data_; }

  void AddType(EventType type) { event_type_ |= type; }
  void RemoveType(EventType type) { event_type_ &= ~type; }
  uint16_t GetType() { return event_type_; }
  void ClearType() { event_type_ = 0; }
  void ForceSetType(EventType type) { event_type_ = type; }

  void SetSocket(std::shared_ptr<Socket> socket) { socket_ = socket; }
  std::shared_ptr<Socket> GetSocket() { return socket_.lock(); }

#ifdef __win__
  void SetBuffer(std::shared_ptr<BufferQueue>& buffer) { _buffer = buffer; }
  std::shared_ptr<BufferQueue> GetBuffer() { return _buffer; }

 private:
  std::shared_ptr<BufferQueue> _buffer;
#endif

 protected:
  void*    data_;
  uint16_t event_type_;
  std::weak_ptr<Socket> socket_;
};

}  // namespace cppnet

#endif  // CPPNET_EVENT_EVENT_INTERFACE_H_
