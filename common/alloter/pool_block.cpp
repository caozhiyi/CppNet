// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#include <cstdlib>
#include <algorithm>
#include "pool_block.h"
#include "cppnet/cppnet_config.h"

namespace cppnet {


BlockMemoryPool::BlockMemoryPool(uint32_t large_sz, uint32_t add_num) :
                                  _number_large_add_nodes(add_num),
                                  _large_size(large_sz){

}

BlockMemoryPool::~BlockMemoryPool() {
#ifdef __use_iocp__
    std::lock_guard<std::mutex> lock(_mutex);
#endif
    // free all memory
    for (auto iter = _free_mem_vec.begin(); iter != _free_mem_vec.end(); ++iter) {
        free(*iter);
    }
    _free_mem_vec.clear();
}

void* BlockMemoryPool::PoolLargeMalloc() {
#ifdef __use_iocp__
    std::lock_guard<std::mutex> lock(_mutex);
#endif
    if (_free_mem_vec.empty()) {
        Expansion();
    }

    void* ret = _free_mem_vec.back();
    _free_mem_vec.pop_back();
    return ret;
}

void BlockMemoryPool::PoolLargeFree(void* &m) {
    bool release = false;
    {
#ifdef __use_iocp__
        std::lock_guard<std::mutex> lock(_mutex);
#endif
        _free_mem_vec.push_back(m);

        if (_free_mem_vec.size() > __max_block_num) {
            release = true;
        }
    }
    
    // release some block.
    if (release) {
        ReleaseHalf();
    }
}

uint32_t BlockMemoryPool::GetSize() {
#ifdef __use_iocp__
    std::lock_guard<std::mutex> lock(_mutex);
#endif
    return (uint32_t)_free_mem_vec.size();
}

uint32_t BlockMemoryPool::GetBlockLength() {
    return _large_size;
}

void BlockMemoryPool::ReleaseHalf() {
#ifdef __use_iocp__
    std::lock_guard<std::mutex> lock(_mutex);
#endif
    size_t size = _free_mem_vec.size();
    size_t hale = size / 2;
    for (auto iter = _free_mem_vec.begin(); iter != _free_mem_vec.end();) {
        void* mem = *iter;

        iter = _free_mem_vec.erase(iter);
        free(mem);
        
        size--;
        if (iter == _free_mem_vec.end() || size <= hale) {
            break;
        }
    }
}

void BlockMemoryPool::Expansion(uint32_t num) {
    if (num == 0) {
        num = _number_large_add_nodes;
    }

    for (uint32_t i = 0; i < num; ++i) {
        void* mem = malloc(_large_size);
        // not memset!
        _free_mem_vec.push_back(mem);
    }
}

std::shared_ptr<BlockMemoryPool> MakeBlockMemoryPoolPtr(uint32_t large_sz, uint32_t add_num) {
    return std::make_shared<BlockMemoryPool>(large_sz, add_num);
}

}