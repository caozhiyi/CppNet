// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)
// Copyright <caozhiyi5@gmail.com>

#include <memory>

#include "include/cppnet_buffer.h"
#include "foundation/buffer/buffer_interface.h"

namespace cppnet {

Buffer::Buffer(std::shared_ptr<fdan::Buffer> buffer): buffer_(buffer) {}

Buffer::~Buffer() {}

uint32_t Buffer::ReadNotMovePt(char* res, uint32_t len) {
  return buffer_->ReadNotMovePt(res, len);
}

uint32_t Buffer::Read(char* res, uint32_t len) {
  return buffer_->Read(res, len);
}

void Buffer::Clear() {
  buffer_->Clear();
}

uint32_t Buffer::ReadUntil(char* res, uint32_t len) {
  return buffer_->ReadUntil(res, len);
}

int32_t Buffer::MoveReadPt(int32_t len) {
  return buffer_->MoveReadPt(len);
}

uint32_t Buffer::ReadUntil(char* res, uint32_t len, const char* find,
  uint32_t find_len, uint32_t& need_len) {
  return buffer_->ReadUntil(res, len, find, find_len, need_len);
}

uint32_t Buffer::GetCanReadLength() {
  return buffer_->GetCanReadLength();
}

uint32_t Buffer::FindStr(const char* s, uint32_t s_len) {
  return buffer_->FindStr(s, s_len);
}

}  // namespace cppnet
