#ifndef COMMON_BUFFER_BUFFER_INTERFACE
#define COMMON_BUFFER_BUFFER_INTERFACE

#include <memory>

namespace cppnet {

class BlockMemoryPool;
class Buffer {
public:
    Buffer() {}
    virtual ~Buffer() {}

    // read to res buf but don't chenge the read point
    // return read size
    virtual uint32_t ReadNotMovePt(char* res, uint32_t len) = 0;

    virtual uint32_t Read(std::shared_ptr<Buffer> buffer, uint32_t len = 0) = 0;
    virtual uint32_t Write(std::shared_ptr<Buffer> buffer, uint32_t len = 0) = 0;

    virtual uint32_t Read(char* res, uint32_t len) = 0;
    virtual uint32_t Write(const char* data, uint32_t len) = 0;
    
    // clear all data
    virtual void Clear() = 0;

    // move read point
    virtual int32_t MoveReadPt(int32_t len) = 0;
    // move write point
    virtual int32_t MoveWritePt(int32_t len) = 0;

    // do not read when buffer less than len. 
    // return len when read otherwise return 0
    virtual uint32_t ReadUntil(char* res, uint32_t len) = 0;
    
    // do not read when can't find specified character.
    // return read bytes when read otherwise return 0
    // when find specified character but res'length is too short, 
    // return 0 and the last param return need length
    virtual uint32_t ReadUntil(char* res, uint32_t len, const char* find, uint32_t find_len, uint32_t& need_len) = 0;
    
    virtual uint32_t GetCanWriteLength() = 0;
    virtual uint32_t GetCanReadLength() = 0;

    // return can read bytes
    virtual uint32_t FindStr(const char* s, uint32_t s_len) = 0;

    // return block memory pool
    virtual std::shared_ptr<BlockMemoryPool> GetBlockMemoryPool() = 0;
};
}

#endif