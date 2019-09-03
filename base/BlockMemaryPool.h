#ifndef HEADER_BASE_BLOCKMMEMARYPOOL
#define HEADER_BASE_BLOCKMMEMARYPOOL

#include <mutex>
#include <vector>

namespace base {

    static const int __align = 8;

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

    	// return bulk memory size
    	int GetSize();

		// release half memory
		void ReleaseHalf();
		void Expansion(int num = 0);

    private:
    	int RoundUp(int size, int align = __align) {
    		return ((size + align - 1) & ~(align - 1));
    	}

    private:
    	std::mutex				_large_mutex;
    	int						_number_large_add_nodes; //everytime add nodes num
    	int						_large_size;			 //bulk memory size
    	std::vector<void*>   	_free_mem_vec;			 //free bulk memory list
		std::vector<void*>   	_used_mem_vec;			 //used bulk memory list
    };
}

#endif