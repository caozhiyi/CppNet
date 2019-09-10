#ifndef HEADER_BASE_BUFFER
#define HEADER_BASE_BUFFER

#include <memory>
#include <vector>
#include <mutex>

namespace base {

    struct iovec {
        void      *iov_base;      // starting address of buffer
        size_t    iov_len;        // size of buffer
        iovec(void* base, size_t len) : iov_base(base), iov_len(len) {}
    };

    class CLoopBuffer;
    class CMemoryPool;
    class CBuffer {
    public:
        CBuffer(std::shared_ptr<CMemoryPool>& pool);
        ~CBuffer();
    
        // read to res buf but don't chenge the read point
        // return read size
        int ReadNotClear(char* res, int len);

        int Read(char* res, int len);
        int Write(const char* str, int len);
        
        // clear all if len = 0
        // or modify read point
        void Clear(int len = 0);

        // move write point
        int MoveWritePt(int len);
    
        // do not read when buffer less than len. 
        // return len when read otherwise return 0
        int ReadUntil(char* res, int len);
    
        // do not read when can't find specified character.
        // return read bytes when read otherwise return 0
        // when find specified character but res'length is too short, 
        // return 0 and the last param return need length
        int ReadUntil(char* res, int len, const char* find, int find_len, int& need_len);
    
        int GetFreeLength();
        int GetCanReadLength();

        // get free memory block, 
        // block_vec: memory block vector.
        // size: count block_vec's memory, bigger than size.
        // if size = 0, return existing free memory block. 
        // return size of free memory. 
        int GetFreeMemoryBlock(std::vector<iovec>& block_vec, int size = 0);

        // get use memory block, 
        // block_vec: memory block vector.
        // return size of use memory. 
        int GetUseMemoryBlock(std::vector<iovec>& block_vec, int max_size = 4096);

        // return can read bytes
        int FindStr(const char* s, int s_len) const;

        friend std::ostream & operator<< (std::ostream &out, const CLoopBuffer &obj);
    
    private:
        void _Reset();
    
    public:
        int          _buff_count;
        CLoopBuffer* _buffer_read;
        CLoopBuffer* _buffer_write;
        CLoopBuffer* _buffer_end;
    
        std::mutex   _mutex;
        std::shared_ptr<CMemoryPool>    _pool;
    };

    std::ostream& operator<<(std::ostream &out, const CBuffer &obj);
}
#endif