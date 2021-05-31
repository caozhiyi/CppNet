// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef COMMON_BUFFER_BUFFER_INTERFACE
#define COMMON_BUFFER_BUFFER_INTERFACE

#include "include/cppnet_buffer.h"

namespace cppnet {

class BlockMemoryPool;
class InnerBuffer:
    public Buffer {

public:
    InnerBuffer() {}
    virtual ~InnerBuffer() {}

    // read to res buf but don't change the read point
    // return read size
    virtual uint32_t ReadNotMovePt(char* res, uint32_t len) = 0;

    virtual uint32_t Read(char* res, uint32_t len) = 0;
    virtual uint32_t Write(const char* data, uint32_t len) = 0;
    
    // clear all data
    virtual void Clear() = 0;

    // do not read when buffer less than len. 
    // return len when read otherwise return 0
    virtual uint32_t ReadUntil(char* res, uint32_t len) = 0;
    
    // do not read when can't find specified character.
    // return read bytes when read otherwise return 0
    // when find specified character but res length is too short, 
    // return 0 and the last param return need length
    virtual uint32_t ReadUntil(char* res, uint32_t len, const char* find, uint32_t find_len, uint32_t& need_len) = 0;
    
    virtual uint32_t GetCanWriteLength() = 0;
    virtual uint32_t GetCanReadLength() = 0;

    // return can read bytes
    virtual uint32_t FindStr(const char* s, uint32_t s_len) = 0;

    virtual uint32_t Read(std::shared_ptr<InnerBuffer> buffer, uint32_t len = 0) = 0;
    virtual uint32_t Write(std::shared_ptr<InnerBuffer> buffer, uint32_t len = 0) = 0;

    // move read point
    virtual int32_t MoveReadPt(int32_t len) = 0;
    // move write point
    virtual int32_t MoveWritePt(int32_t len) = 0;

    // return block memory pool
    virtual std::shared_ptr<BlockMemoryPool> GetBlockMemoryPool() = 0;
};

}

#endif