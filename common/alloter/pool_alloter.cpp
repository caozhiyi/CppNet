#include <cstring>
#include <cstdlib>

#include "pool_alloter.h"
#include "normal_alloter.h"

namespace cppnet {


PoolAlloter::PoolAlloter() : 
    _pool_start(nullptr),
    _pool_end(nullptr) {
    _free_list.resize(__default_number_of_free_lists);
    memset(&(*_free_list.begin()), 0, sizeof(void*) * __default_number_of_free_lists);
    _alloter = MakeNormalAlloterPtr();
}

PoolAlloter::~PoolAlloter() {
    for (auto iter = _malloc_vec.begin(); iter != _malloc_vec.end(); ++iter) {
        if (*iter) {
            void* data = (void*)*iter;
            _alloter->Free(data);
        }
    }
}

void* PoolAlloter::Malloc(uint32_t size) {
    if (size > __default_max_bytes) {
        void* ret = _alloter->Malloc(size);
        return ret;
    }
    
    MemNode** my_free = &(_free_list[FreeListIndex(size)]);
    MemNode* result = *my_free;
    if (result == nullptr) {
        void* bytes = ReFill(Align(size));
        return bytes;
    }
    
    *my_free = result->_next;
    return result;
}

void* PoolAlloter::MallocAlign(uint32_t size) {
    return Malloc(Align(size));
}

void* PoolAlloter::MallocZero(uint32_t size) {
    void* ret = Malloc(size);
    if (ret) {
        memset(ret, 0, size);
    }
    return ret;
}

void PoolAlloter::Free(void* &data, uint32_t len) {
    if (!data) {
        return;
    }
    
    if (len > __default_max_bytes) {
        _alloter->Free(data);
        data = nullptr;
        return;
    }
    
    MemNode* node = (MemNode*)data;
    MemNode** my_free = &(_free_list[FreeListIndex(len)]);
    
    node->_next = *my_free;
    *my_free = node;
    data = nullptr;
}

void* PoolAlloter::ReFill(uint32_t size, uint32_t num) {
    uint32_t nums = num;

    char* chunk = (char*)ChunkAlloc(size, nums);

    MemNode* volatile* my_free;
    MemNode* res, *current, *next;
    if (1 == nums) {
        return chunk;
    }

    res = (MemNode*)chunk;
    
    my_free = &(_free_list[FreeListIndex(size)]);

    *my_free = next = (MemNode*)(chunk + size);
    for (uint32_t i = 1;; i++) {
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

void* PoolAlloter::ChunkAlloc(uint32_t size, uint32_t& nums) {
    char* res;
    uint32_t need_bytes = size * nums;
    uint32_t left_bytes = uint32_t(_pool_end - _pool_start);

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
    uint32_t bytes_to_get = size * nums;

    if (left_bytes > 0) {
        MemNode* my_free = _free_list[FreeListIndex(left_bytes)];
        ((MemNode*)_pool_start)->_next = my_free;
        _free_list[FreeListIndex(size)] = (MemNode*)_pool_start;
    }

    _pool_start = (char*)_alloter->Malloc(bytes_to_get);

    _malloc_vec.push_back(_pool_start);
    _pool_end = _pool_start + bytes_to_get;
    return ChunkAlloc(size, nums);
}

std::shared_ptr<Alloter> MakePoolAlloterPtr() {
    return std::make_shared<cppnet::PoolAlloter>();
}

}