// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#include <cstdlib>
#include <algorithm>
#include "pool_block.h"

namespace cppnet {


BlockMemoryPool::BlockMemoryPool(uint32_t large_sz, uint32_t add_num) :
                                  _number_large_add_nodes(add_num),
                                  _large_size(large_sz){

}

BlockMemoryPool::~BlockMemoryPool() {
    // free all memory
    for (auto iter = _all_mem_vec.begin(); iter != _all_mem_vec.end(); ++iter) {
        free(*iter);
    }
}

void* BlockMemoryPool::PoolLargeMalloc() {
    if (_free_mem_vec.empty()) {
        Expansion();
    }

    void* ret = _free_mem_vec.back();
    _free_mem_vec.pop_back();
    return ret;
}

void BlockMemoryPool::PoolLargeFree(void* &m) {
    _free_mem_vec.push_back(m);
}

uint32_t BlockMemoryPool::GetSize() {
    return (uint32_t)_free_mem_vec.size();
}

uint32_t BlockMemoryPool::GetBlockLength() {
    return _large_size;
}

void BlockMemoryPool::ReleaseHalf() {
    size_t size = _free_mem_vec.size();
    size_t hale = size / 2;
    for (auto iter = _free_mem_vec.begin(); iter != _free_mem_vec.end();) {
        void* mem = *iter;

        auto all_iter = std::find(_all_mem_vec.begin(), _all_mem_vec.end(), mem);
        if (all_iter != _all_mem_vec.end()) {
            _all_mem_vec.erase(all_iter);
        }
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
        _all_mem_vec.push_back(mem);
    }
}

std::shared_ptr<BlockMemoryPool> MakeBlockMemoryPoolPtr(uint32_t large_sz, uint32_t add_num) {
    return std::make_shared<BlockMemoryPool>(large_sz, add_num);
}

}