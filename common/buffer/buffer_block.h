// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef COMMON_BUFFER_BUFFER_BLOCK
#define COMMON_BUFFER_BUFFER_BLOCK

#include <memory>
#include "common/structure/list_slot.h"
#include "common/buffer/buffer_interface.h"

namespace cppnet {

class BlockMemoryPool;
class BufferBlock: 
    public InnerBuffer, 
    public ListSlot<BufferBlock> {

public:
    BufferBlock(std::shared_ptr<BlockMemoryPool>& alloter);
    ~BufferBlock();

    // read to res buf but don't change the read point
    // return read size
    uint32_t ReadNotMovePt(char* res, uint32_t len);

    uint32_t Read(std::shared_ptr<InnerBuffer> buffer, uint32_t len = 0);
    uint32_t Write(std::shared_ptr<InnerBuffer> buffer, uint32_t len = 0);

    uint32_t Read(char* res, uint32_t len);
    uint32_t Write(const char* data, uint32_t len);
    
    // clear all data
    void Clear();

    // move read point
    int32_t MoveReadPt(int32_t len);
    // move write point
    int32_t MoveWritePt(int32_t len);

    // do not read when buffer less than len. 
    // return len when read otherwise return 0
    uint32_t ReadUntil(char* res, uint32_t len);
    
    // do not read when can't find specified character.
    // return read bytes when read otherwise return 0
    // when find specified character but res length is too short, 
    // return 0 and the last param return need length
    uint32_t ReadUntil(char* res, uint32_t len, const char* find, uint32_t find_len, uint32_t& need_len);
    
    uint32_t GetCanWriteLength();
    uint32_t GetCanReadLength();

    // get free memory block, 
    // res1: point to memory of start.
    // len1: length of memory.
    // there may be two blocks
    bool GetFreeMemoryBlock(void*& res1, uint32_t& len1, void*& res2, uint32_t& len2);

    // get used memory block, 
    // res1: point to memory of start.
    // len1: length of memory.
    // there may be two blocks
    bool GetUseMemoryBlock(void*& res1, uint32_t& len1, void*& res2, uint32_t& len2);

    // return can read bytes
    uint32_t FindStr(const char* s, uint32_t s_len);

    // return block memory pool
    std::shared_ptr<BlockMemoryPool> GetBlockMemoryPool();

private:
    //find str in fix length buffer. return the first position if find otherwise return nullptr
    const char* _FindStrInMem(const char* buffer, const char* ch, uint32_t buffer_len, uint32_t ch_len) const;
    uint32_t _Read(char* res, uint32_t len, bool move_pt);
    uint32_t _Write(const char* data, uint32_t len);

private:
    uint32_t _total_size;       //total buffer size
    char*    _read;             //read position
    char*    _write;            //write position
    char*    _buffer_start;
    char*    _buffer_end;
    bool     _can_read;         //when _read == _write? Is there any data can be read.
    std::weak_ptr<BlockMemoryPool> _alloter;
};
}

#endif