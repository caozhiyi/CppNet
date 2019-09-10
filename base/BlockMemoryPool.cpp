#include "BlockMemoryPool.h"

using namespace base;

CBlockMemoryPool::CBlockMemoryPool(const int large_sz, const int add_num) : 
                                  _large_size(large_sz), 
                                  _number_large_add_nodes(add_num) {

}

CBlockMemoryPool::~CBlockMemoryPool() {
    // free all memory
    std::unique_lock<std::mutex> lock(_large_mutex);
    for (auto iter = _free_mem_vec.begin(); iter != _free_mem_vec.end(); ++iter) {
        free(*iter);
    }
}

void* CBlockMemoryPool::PoolLargeMalloc() {
    if (_free_mem_vec.empty()) {
        Expansion();
    }
    
    std::unique_lock<std::mutex> lock(_large_mutex);
    void* ret = _free_mem_vec.back();
    _free_mem_vec.pop_back();
    return ret;
}

void CBlockMemoryPool::PoolLargeFree(void* &m) {
    std::unique_lock<std::mutex> lock(_large_mutex);
    _free_mem_vec.push_back(m);
}

int CBlockMemoryPool::GetSize() {
    std::unique_lock<std::mutex> lock(_large_mutex);
    return (int)_free_mem_vec.size();
}

int CBlockMemoryPool::GetBlockLength() {
    return _large_size;
}

void CBlockMemoryPool::ReleaseHalf() {
    std::unique_lock<std::mutex> lock(_large_mutex);
    size_t size = _free_mem_vec.size();
    size_t hale = size / 2;
    for (auto iter = _free_mem_vec.begin(); iter != _free_mem_vec.end(); ++iter) {
        size--;
        if (size <= hale) {
            break;
        }
        void* mem = *iter;
        iter = _free_mem_vec.erase(iter);
        free(mem);
    }
}

void CBlockMemoryPool::Expansion(int num) {
    if (num == 0) {
        num = _number_large_add_nodes;
    }

    std::unique_lock<std::mutex> lock(_large_mutex);
    for (int i = 0; i < num; ++i) {
        void* mem = malloc(_large_size);
        // not memset!
        _free_mem_vec.push_back(mem);
    }
}