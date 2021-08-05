// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)
// Copyright <caozhiyi5@gmail.com>

#ifndef INCLUDE_CPPNET_BUFFER_H_
#define INCLUDE_CPPNET_BUFFER_H_

#include <memory>
#include <cstdint>

namespace fdan {
class Buffer;
}

namespace cppnet {

class Buffer {
 public:
  explicit Buffer(std::shared_ptr<fdan::Buffer> buffer);
  virtual ~Buffer();

  // read to res buf but don't change the read point
  // return read size
  virtual uint32_t ReadNotMovePt(char* res, uint32_t len);

  virtual uint32_t Read(char* res, uint32_t len);

  // clear all data
  virtual void Clear();

  // do not read when buffer less than len.
  // return len when read otherwise return 0
  virtual uint32_t ReadUntil(char* res, uint32_t len);

  // move read point
  virtual int32_t MoveReadPt(int32_t len);

  // do not read when can't find specified character.
  // return read bytes when read otherwise return 0
  // when find specified character but res length is too short,
  // return 0 and the last param return need length
  virtual uint32_t ReadUntil(char* res, uint32_t len, const char* find,
    uint32_t find_len, uint32_t& need_len);

  virtual uint32_t GetCanReadLength();

  // return can read bytes
  virtual uint32_t FindStr(const char* s, uint32_t s_len);

 private:
  std::shared_ptr<fdan::Buffer> buffer_;
};

}  // namespace cppnet

#endif  // INCLUDE_CPPNET_BUFFER_H_
