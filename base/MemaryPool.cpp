#include <assert.h>
#include "MemaryPool.h"

CMemaryPool::CMemaryPool() {
	for (int i = 0; i < __number_of_free_lists; i++) {
		_free_list[i] = nullptr;
	}
	_create_thread_id = std::this_thread::get_id();
}

CMemaryPool::CMemaryPool(const int large_sz, const int add_num) : _large_size(RoundUp(large_sz)), _number_large_add_nodes(add_num){
	for (int i = 0; i < __number_of_free_lists; i++) {
		_free_list[i] = nullptr;
	}
	_create_thread_id = std::this_thread::get_id();
}

CMemaryPool::~CMemaryPool() {
	//assert(_create_thread_id == std::this_thread::get_id());
	for (auto iter = _malloc_vec.begin(); iter != _malloc_vec.end(); ++iter) {
		if (*iter) {
			free(*iter);
		}
	}
}

std::thread::id CMemaryPool::GetCreateThreadId() {
	return _create_thread_id;
}

int CMemaryPool::GetLargeSize() const {
	return _large_size;
}

void* CMemaryPool::ReFill(int size, int num, bool is_large) {
	int nums = num;

	char* chunk = (char*)ChunkAlloc(size, nums);
	MemNode** my_free;
	MemNode* res, *current, *next;
	if (1 == nums) {
		return chunk;
	}

	res = (MemNode*)chunk;
	
	if (is_large) {
		if (_free_large.find(size) == _free_large.end()) {
			_free_large[size] = nullptr;
		}
		my_free = &_free_large[size];

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

	} else {
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
	}
	return res;
}

void* CMemaryPool::ChunkAlloc(int size, int& nums, bool is_large) {
	char* res;
	int need_bytes = size * nums;
	int left_bytes = _pool_end - _pool_start;

	//内存池够用
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

	if (!is_large) {
		if (left_bytes > 0) {
			MemNode* my_free = _free_list[FreeListIndex(left_bytes)];
			((MemNode*)_pool_start)->_next = my_free;
			_free_list[FreeListIndex(size)] = (MemNode*)_pool_start;
		}

	} else {
		free(_pool_start);
	}
	

	_pool_start = (char*)malloc(bytes_to_get);
	//内存分配失败
	if (0 == _pool_start) {
		throw std::exception(std::logic_error("There memary is not enough!"));
		return nullptr;
	}

	_malloc_vec.push_back(_pool_start);
	_pool_end = _pool_start + bytes_to_get;
	return ChunkAlloc(size, nums, is_large);
}