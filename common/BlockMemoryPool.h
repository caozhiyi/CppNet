#ifndef HEADER_BASE_BLOCKMMEMORYPOOL
#define HEADER_BASE_BLOCKMMEMORYPOOL

#include <mutex>
#include <vector>

namespace cppnet {

// all memory must return memory pool before destory.
class BlockMemoryPool {
public:
    // bulk memory size. everytime add nodes num
    BlockMemoryPool(uint32_t large_sz, uint32_t add_num);
    ~BlockMemoryPool();
        
    // for bulk memory. 
    // return one bulk memory node
    void* PoolLargeMalloc();
    void PoolLargeFree(void* &m);

    // return bulk memory list size
    uint32_t GetSize();
    // return length of bulk memory
    uint32_t GetBlockLength();

    // release half memory
    void ReleaseHalf();
    void Expansion(uint32_t num = 0);

private:
    std::mutex                _large_mutex;
    uint32_t                  _number_large_add_nodes; //every time add nodes num
    uint32_t                  _large_size;             //bulk memory size
    std::vector<void*>        _free_mem_vec;           //free bulk memory list
};

}

#endif
