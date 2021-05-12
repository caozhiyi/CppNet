// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef COMMON_ALLOTER_POOL_BLOCK
#define COMMON_ALLOTER_POOL_BLOCK

#include <vector>
#include <memory>
#include <cstdint>

namespace cppnet {

// all memory must return memory pool before destory.
class BlockMemoryPool {
public:
    // bulk memory size. 
    // everytime add nodes num
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
    uint32_t                  _number_large_add_nodes; //every time add nodes num
    uint32_t                  _large_size;             //bulk memory size
    std::vector<void*>        _free_mem_vec;           //free bulk memory list
};

std::shared_ptr<BlockMemoryPool> MakeBlockMemoryPoolPtr(uint32_t large_sz, uint32_t add_num);

}

#endif