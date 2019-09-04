#ifndef HEADER_BASE_MMEMARYPOOL
#define HEADER_BASE_MMEMARYPOOL

#include <new>
#include <functional>
#include <thread>
#include <mutex>
#include <vector>
#include <map>
#include <cstring>		//for memset
#include <stdexcept>	//for logic_error

#include "BlockMemaryPool.h"

namespace base {

    static const int __align = 8;
    static const int __max_bytes = 256;
    static const int __number_of_free_lists = __max_bytes / __align;
    static const int __number_add_nodes = 20;

    class CMemoryPool {
    public:
    	// bulk memory size. 
        // everytime add nodes num
    	CMemoryPool(const int large_sz, const int add_num);
    	~CMemoryPool();
    
    	//for object. invocation of constructors and destructors
    	template<typename T, typename... Args >
    	T* PoolNew(Args&&... args);
    	template<typename T>
    	void PoolDelete(T* &c);
    
    	//for continuous memory
    	template<typename T>
    	T* PoolMalloc(int size);
    	template<typename T>
    	void PoolFree(T* &m, int len);
    
    	//for bulk memory. 
    	//return one bulk memory node
    	template<typename T>
    	T* PoolLargeMalloc();
    	template<typename T>
    	void PoolLargeFree(T* &m);
    
    	std::thread::id GetCreateThreadId();
    
    	//return bulk memory size
    	int GetLargeSize();

        // release half memory
        void ReleaseLargeHalf();

        void ExpansionLarge(int num = 0);
    
    private:
    	int RoundUp(int size, int align = __align) {
    		return ((size + align - 1) & ~(align - 1));
    	}
    
    	int FreeListIndex(int size, int align = __align) {
    		return (size + align - 1) / align - 1;
    	}
    
    	void* ReFill(int size, int num = __number_add_nodes);
    	void* ChunkAlloc(int size, int& nums);
    	
    private:
    	union MemNode {
    		MemNode*	_next;
    		char		_data[1];
    	};
    
    	MemNode*	_free_list[__number_of_free_lists];	
    	char*		_pool_start;					
    	char*		_pool_end;				
    	std::thread::id			_create_thread_id;
    	std::vector<char*>		_malloc_vec;
    	std::recursive_mutex	_mutex;

        CBlockMemoryPool        _block_pool;
    };
    
    template<typename T, typename... Args>
    T* CMemoryPool::PoolNew(Args&&... args) {
    	int sz = sizeof(T);
    	if (sz > __max_bytes) {
    		void* bytes = malloc(sz);
    		T* res = new(bytes) T(std::forward<Args>(args)...);
    		return res;
    	}
    
    	std::unique_lock<std::recursive_mutex> lock(_mutex);
    	MemNode** my_free = &(_free_list[FreeListIndex(sz)]);
    	MemNode* result = *my_free;
    	if (result == nullptr) {
    		void* bytes = ReFill(RoundUp(sz));
    		T* res = new(bytes) T(std::forward<Args>(args)...);
    		return res;
    	}
    	*my_free = result->_next;
    	T* res = new(result) T(std::forward<Args>(args)...);
    	return res;
    }
    
    template<typename T>
    void CMemoryPool::PoolDelete(T* &c) {
    	if (!c) {
    		return;
    	}
    
    	int sz = sizeof(T);
    	if (sz > __max_bytes) {
    		c->~T();
    		free(c);
    		return;
    	}
    
    	MemNode* node = (MemNode*)c;
    	std::unique_lock<std::recursive_mutex> lock(_mutex);
    	MemNode** my_free = &(_free_list[FreeListIndex(sz)]);
    
    	c->~T();
    	node->_next = *my_free;
    	*my_free = node;
    	c = nullptr;
    }
    
    template<typename T>
    T* CMemoryPool::PoolMalloc(int sz) {
    	if (sz > __max_bytes) {
    		void* bytes = malloc(sz);
    		memset(bytes, 0, sz);
    		return (T*)bytes;
    	}
    
    	std::unique_lock<std::recursive_mutex> lock(_mutex);
    	MemNode** my_free = &(_free_list[FreeListIndex(sz)]);
    	MemNode* result = *my_free;
    	if (result == nullptr) {
    		void* bytes = ReFill(RoundUp(sz));
    		memset(bytes, 0, sz);
    		return (T*)bytes;
    	}
    
    	*my_free = result->_next;
    	memset(result, 0, sz);
    	return (T*)result;
    }
    
    template<typename T>
    void CMemoryPool::PoolFree(T* &m, int len) {
    	if (!m) {
    		return;
    	}
    
    	if (len > __max_bytes) {
    		free(m);
    		m = nullptr;
    		return;
    	}
    
    	MemNode* node = (MemNode*)m;
    	std::unique_lock<std::recursive_mutex> lock(_mutex);
    	MemNode** my_free = &(_free_list[FreeListIndex(len)]);
    
    	node->_next = *my_free;
    	*my_free = node;
    	m = nullptr;
    }
    
    template<typename T>
    T* CMemoryPool::PoolLargeMalloc() {
        T* ret = (T*)_block_pool.PoolLargeMalloc();
        return ret;
    }
    
    template<typename T>
    void CMemoryPool::PoolLargeFree(T* &m) {
    	if (!m) {
    		return;
    	}
    	
        _block_pool.PoolLargeFree((void*&)m);
        m = nullptr;
    }
}

#endif