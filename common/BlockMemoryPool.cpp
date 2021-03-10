#include "BlockMemoryPool.h"

 namespace cppnet {

BlockMemoryPool::BlockMemoryPool(uint32_t large_sz,uint32_t add_num):
    _number_large_add_nodes(add_num),
    _large_size(large_sz) {

}

BlockMemoryPool::~BlockMemoryPool() {
    // free all memory
    std::unique_lock<std::mutex> lock(_large_mutex);
    for (auto iter = _free_mem_vec.begin(); iter != _free_mem_vec.end(); ++iter) {
        free(*iter);
    }
}

void* BlockMemoryPool::PoolLargeMalloc() {
    std::unique_lock<std::mutex> lock(_large_mutex);
    if (_free_mem_vec.empty()) {
        Expansion();
    }

    void* ret = _free_mem_vec.back();
    _free_mem_vec.pop_back();
    return ret;
}

void BlockMemoryPool::PoolLargeFree(void* &m) {
    std::unique_lock<std::mutex> lock(_large_mutex);
    _free_mem_vec.push_back(m);
}

uint32_t BlockMemoryPool::GetSize() {
    std::unique_lock<std::mutex> lock(_large_mutex);
    return (uint32_t)_free_mem_vec.size();
}

uint32_t BlockMemoryPool::GetBlockLength() {
    return _large_size;
}

void BlockMemoryPool::ReleaseHalf() {
    std::unique_lock<std::mutex> lock(_large_mutex);
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

}