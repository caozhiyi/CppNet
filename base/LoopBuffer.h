#ifndef HEADER_BASE_LOOPBUFFER
#define HEADER_BASE_LOOPBUFFER

#include <mutex>
#include <memory>

namespace base {

    class CMemoryPool;
    class CLoopBuffer {
    public:
        CLoopBuffer(std::shared_ptr<CMemoryPool>& pool);
        ~CLoopBuffer();
    
        // read to res buf but don't chenge the read point
        // return read size
        int ReadNotClear(char* res, int len);

        int Read(char* res, int len);
        int Write(const char* str, int len);
        
        // clear all if len = 0
        // or modify read point
        int Clear(int len = 0);
    
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
        // res1: point to memory fo start.
        // len1: length of memory.
        // there may be two blocks
        bool GetFreeMemoryBlock(void*& res1, int& len1, void*& res2, int& len2);

        // get used memory block, 
        // res1: point to memory fo start.
        // len1: length of memory.
        // there may be two blocks
        bool GetUseMemoryBlock(void*& res1, int& len1, void*& res2, int& len2);

        // return can read bytes
        int FindStr(const char* s, int s_len);

        // list point
        CLoopBuffer* GetNext();
        void SetNext(CLoopBuffer* next);
    
        friend std::ostream & operator<< (std::ostream &out, const CLoopBuffer &obj);
    
    private:
        //find str in fix length buffer. return the first pos if find otherwise return nullptr
        const char* _FindStrInMem(const char* buffer, const char* ch, int buffer_len, int ch_len) const;
        int _Read(char* res, int len, bool clear);
        int _Write(const char* str, int len, bool write);
    
    private:
        int      _total_size;       //total buffer size
        char*    _read;             //read pos
        char*    _write;            //write pos
        char*    _buffer_start;
        char*    _buffer_end;
        bool     _can_read;         //when _read == _write? Is there any data can be read.
        std::mutex   _mutex;
        CLoopBuffer* _next;         //point to next node
        std::shared_ptr<CMemoryPool>    _pool;
    };

    std::ostream & operator<< (std::ostream &out, const CLoopBuffer &obj);
}

#endif