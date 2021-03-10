#include <assert.h>
#include "Log.h"
#include "MemoryPool.h"

namespace cppnet {

MemoryPool::MemoryPool(const int large_sz, const int add_num) : _block_pool(RoundUp(large_sz), add_num) {
    for (int i = 0; i < __number_of_free_lists; i++) {
        _free_list[i] = nullptr;
    }
    _pool_start = nullptr;
    _pool_end = nullptr;
    _create_thread_id = std::this_thread::get_id();
}

MemoryPool::~MemoryPool() {
    //assert(_create_thread_id == std::this_thread::get_id());
    for (auto iter = _malloc_vec.begin(); iter != _malloc_vec.end(); ++iter) {
        if (*iter) {
            free(*iter);
        }
    }
}

std::thread::id MemoryPool::GetCreateThreadId() {
    return _create_thread_id;
}

int MemoryPool::GetLargeSize() {
    return _block_pool.GetSize();
}

int MemoryPool::GetLargeBlockLength() {
    return _block_pool.GetBlockLength();
}

void MemoryPool::ReleaseLargeHalf() {
    _block_pool.ReleaseHalf();
}

void MemoryPool::ExpansionLarge(int num) {
    _block_pool.Expansion(num);
}

void* MemoryPool::ReFill(int size, int num) {
    int nums = num;

    char* chunk = nullptr;
    try {
        chunk = (char*)ChunkAlloc(size, nums);

    } catch (const std::exception& e) {
        LOG_FATAL("malloc memory failed! info : %s", e.what());
        abort();
    }
    
    MemNode* volatile* my_free;
    MemNode* res, *current, *next;
    if (1 == nums) {
        return chunk;
    }

    res = (MemNode*)chunk;
    
    my_free = &(_free_list[FreeListIndex(size)]);

    *my_free = next = (MemNode*)(chunk + size);
    for (int i = 1;; i++) {
        current = next;
        next = (MemNode*)((char*)next + size);
        if (nums - 1 == i) {
            current->_next = nullptr;
            break;

        } else {
            current->_next = next;
        }
    }
    return res;
}

void* MemoryPool::ChunkAlloc(int size, int& nums) {
    char* res;
    int need_bytes = size * nums;
    int left_bytes = _pool_end - _pool_start;

    //pool is enough
    if (left_bytes >= need_bytes) {
        res = _pool_start;
        _pool_start += need_bytes;
        return res;
    
    } else if (left_bytes >= size) {
        nums = left_bytes / size;
        need_bytes = size * nums;
        res = _pool_start;
        _pool_start += need_bytes;
        return res;

    } 
    int bytes_to_get = size * nums;

    if (left_bytes > 0) {
        MemNode* my_free = _free_list[FreeListIndex(left_bytes)];
        ((MemNode*)_pool_start)->_next = my_free;
        _free_list[FreeListIndex(size)] = (MemNode*)_pool_start;
    }

    _pool_start = (char*)malloc(bytes_to_get);
    //malloc failed
    if (0 == _pool_start) {
        throw std::exception(std::logic_error("There memory is not enough!"));
        //cout << "There memory is not enough!" << endl;
        return nullptr;
    }

    _malloc_vec.push_back(_pool_start);
    _pool_end = _pool_start + bytes_to_get;
    return ChunkAlloc(size, nums);
}

}