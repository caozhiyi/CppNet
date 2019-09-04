#ifndef HEADER_BASE_BLOCKMMEMARYPOOL
#define HEADER_BASE_BLOCKMMEMARYPOOL

#include <mutex>
#include <vector>

namespace base {

	// all memory must return memory pool before destory.
    class CBlockMemoryPool {
    public:
    	// bulk memory size. everytime add nodes num
    	CBlockMemoryPool(const int large_sz, const int add_num);
    	~CBlockMemoryPool();
    	
    	// for bulk memory. 
    	// return one bulk memory node
    	void* PoolLargeMalloc();
    	void PoolLargeFree(void* &m);

    	// return bulk memory list size
    	int GetSize();
		// return length of bulk memory
		int GetBlockLength();

		// release half memory
		void ReleaseHalf();
		void Expansion(int num = 0);

    private:
    	std::mutex				_large_mutex;
    	int						_number_large_add_nodes; //everytime add nodes num
    	int						_large_size;			 //bulk memory size
    	std::vector<void*>   	_free_mem_vec;			 //free bulk memory list
		std::vector<void*>   	_used_mem_vec;			 //used bulk memory list
    };
}

#endif