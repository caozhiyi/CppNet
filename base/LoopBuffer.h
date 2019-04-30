#ifndef HEADER_LOOPBUFFER
#define HEADER_LOOPBUFFER
#include <memory>
#include <mutex>
class CMemoryPool;
class CLoopBuffer {
public:
	//maybe throw exception
	explicit CLoopBuffer(std::shared_ptr<CMemoryPool>& pool, int index);
	explicit CLoopBuffer(std::shared_ptr<CMemoryPool>& pool, int size, int index);
	~CLoopBuffer();

	//read to res buf not chenge the cursor
	int ReadNotClear(char* res, int len);
	int Read(char* res, int len);
	int Write(char* str, int len);
	void Clear();
	//forward moving cursor
	int Clear(int len);

	//do not read when buffer less than len. return len when read otherwise return 0
	int ReadUntil(char* res, int len);

	//do not read when can't find specified character.
	//return read bytes when read otherwise return 0
	//when find specified character but res'length is too short, return 0 and the last param return need length
	int ReadUntil(char* res, int len, const char* find, int find_len, int& need_len);

	int GetFreeSize() const;
	int GetCanReadSize() const;
	CLoopBuffer* GetNext() const;
	void SetNext(CLoopBuffer* next);
	int GetIndex() const;
	char* GetWrite() { return _write; }

	//return can read bytes
	int FindStr(const char* s, int s_len) const;

	void IncrefIndex(int step = 1);
	void DecrefIndex(int step = 1);
	bool CheckUnused() const;

	friend bool operator<(const CLoopBuffer& buf1, const CLoopBuffer& buf2);
	friend bool operator>(const CLoopBuffer& buf1, const CLoopBuffer& buf2);
	friend bool operator<=(const CLoopBuffer& buf1, const CLoopBuffer& buf2);
	friend bool operator>=(const CLoopBuffer& buf1, const CLoopBuffer& buf2);
	friend bool operator==(const CLoopBuffer& buf1, const CLoopBuffer& buf2);
	friend bool operator!=(const CLoopBuffer& buf1, const CLoopBuffer& buf2);
	friend std::ostream & operator<< (std::ostream &out, const CLoopBuffer &obj);

private:
	//find str in fix length buffer. return the first pos if find otherwise return nullptr
	const char* _FindStrInMem(const char* buffer, const char* ch, int buffer_len, int ch_len) const;

private:
	int		_total_size;	//total buffer size
	char*	_read;			//read pos
	char*	_write;			//write pos
	char*	_buffer_start;
	char*	_buffer_end;
	bool	_can_read;		//when _read == _write£¬Is there any data can be read.
	int		_index;			//buffer use it. compare CLoopBuffers
	std::mutex _mutex;

	CLoopBuffer*					_next;		//point to next node
	std::shared_ptr<CMemoryPool>	_pool;
};
#endif

