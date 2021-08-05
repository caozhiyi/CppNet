// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)
// Copyright <caozhiyi5@gmail.com>

#include <memory>
#include <string>

#include "include/cppnet.h"

#include "cppnet/cppnet_base.h"
#include "cppnet/cppnet_config.h"

#include "foundation/log/log.h"
#include "foundation/log/file_logger.h"
#include "foundation/log/stdout_logger.h"

namespace cppnet {

CppNet::CppNet() {}

CppNet::~CppNet() {
  if (cppnet_base_) {
    cppnet_base_->Dealloc();
  }
}

void CppNet::Init(int32_t thread_num) {
  if (!cppnet_base_) {
    cppnet_base_ = std::make_shared<CppNetBase>();

  } else {
    return;
  }

  cppnet_base_->Init(thread_num);
  if (__print_log) {
    std::shared_ptr<fdan::Logger> file_log =
      std::make_shared<fdan::FileLogger>(__log_file_name);
    std::shared_ptr<fdan::Logger> std_log =
      std::make_shared<fdan::StdoutLogger>();
    file_log->SetLogger(std_log);
    fdan::LOG_SET(file_log);
    fdan::LOG_SET_LEVEL((fdan::LogLevel)__log_level);
  } else {
    fdan::LOG_SET_LEVEL(fdan::LL_NULL);
  }
}

void CppNet::Destory() {
  cppnet_base_->Dealloc();
}

void CppNet::Join() {
  cppnet_base_->Join();
}

void CppNet::SetReadCallback(const read_call_back& cb) {
  cppnet_base_->SetReadCallback(cb);
}

void CppNet::SetWriteCallback(const write_call_back& cb) {
  cppnet_base_->SetWriteCallback(cb);
}

void CppNet::SetDisconnectionCallback(const connect_call_back& cb) {
  cppnet_base_->SetDisconnectionCallback(cb);
}

void CppNet::SetTimerCallback(const timer_call_back& cb) {
  cppnet_base_->SetTimerCallback(cb);
}

uint64_t CppNet::AddTimer(int32_t interval, const user_timer_call_back& cb,
  void* param, bool always) {
  return cppnet_base_->AddTimer(interval, cb, param, always);
}

void CppNet::RemoveTimer(uint64_t timer_id) {
  cppnet_base_->RemoveTimer(timer_id);
}

void CppNet::SetAcceptCallback(const connect_call_back& cb) {
  cppnet_base_->SetAcceptCallback(cb);
}

bool CppNet::ListenAndAccept(const std::string& ip, uint16_t port) {
  return cppnet_base_->ListenAndAccept(ip, port);
}

void CppNet::SetConnectionCallback(const connect_call_back& cb) {
  cppnet_base_->SetConnectionCallback(cb);
}

bool CppNet::Connection(const std::string& ip, uint16_t port) {
  return cppnet_base_->Connection(ip, port);
}

}  // namespace cppnet
