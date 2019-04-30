#ifndef HEADER_BUFFER
#define HEADER_BUFFER

#include <memory>
#include <vector>
#include <mutex>

#define MAX_BUFFER_LEN 8192

//when CLoopBuffer's num more than it. every time do read function will check 
//CLoopBuffer whether be used, if not, release it;
static const int __max_node_size = 12;	

class CLoopBuffer;
class CMemoryPool;
class CBuffer
{
public:
	CBuffer(std::shared_ptr<CMemoryPool>& pool);
	~CBuffer();

	//read to res buf not chenge the cursor
	int ReadNotClear(char* res, int len);
	int Read(char* res, int len);
	int Write(char* str, int len);
	void Clear();
	//forward moving cursor
	void Clear(int len);

	//do not read when buffer less than len. return len when read otherwise return 0
	int ReadUntil(char* res, int len);

	//do not read when can't find specified character.
	//return len when read otherwise return 0
	//when find specified character but res'length is too short, 
	//return 0 and the last param return need length
	int ReadUntil(char* res, int len, const char* find, int find_len, int& need_len);

	int GetFreeSize() const;
	int GetCanReadSize() const;

	//return can read bytes
	int FindStr(const char* s, int s_len) const;

	//release unuse buffer node, but start end read write point node won't be.
	void ReleaseUnuseBuffer();

	//get a buffer or a list that free size more than MAX_BUFFER_LEN
	std::vector<CLoopBuffer*> GetMaxCatch(int size = MAX_BUFFER_LEN);
	std::vector<CLoopBuffer*> GetReadBuffer();

	friend std::ostream& operator<<(std::ostream &out, const CBuffer &obj);

private:
	//modify cloopbuffer index for compare
	void _IncrefIndex(CLoopBuffer* start);
	void _DecrefIndex(CLoopBuffer* start);

public:
	int			 _buffer_num;	//num of CLoopBuffer
	CLoopBuffer* _buffer_start;
	CLoopBuffer* _buffer_end;
	CLoopBuffer* _buffer_read;
	CLoopBuffer* _buffer_write;

	std::mutex	 _mutex;
	std::shared_ptr<CMemoryPool>	_pool;
};
#endif