#include <iostream>
#include "Buffer.h"
#include "MemaryPool.h"
#include "LoopBuffer.h"
#include "Log.h"

using namespace base;

CBuffer::CBuffer(std::shared_ptr<CMemoryPool>& pool) : 
	_pool(pool), 
	_buffer_num(0),
	_buffer_end(nullptr),
	_buffer_read(nullptr),
	_buffer_write(nullptr) {

}

CBuffer::~CBuffer() {
	CLoopBuffer* temp = _buffer_read;
	while (temp) {
		_buffer_read = _buffer_read->GetNext();
		_pool->PoolDelete<CLoopBuffer>(temp);
		temp = _buffer_read;
	}
}

int CBuffer::ReadNotClear(char* res, int len) {
	if (!_buffer_read) {
		return 0;
	}

	std::unique_lock<std::mutex> lock(_mutex);
	CLoopBuffer* temp = _buffer_read;
	int cur_len = 0;
	while (temp && cur_len < len) {
		cur_len += temp->ReadNotClear(res, len - cur_len);
		if (temp == _buffer_write) {
			break;
		}
		temp = temp->GetNext();
	}
	return cur_len;
}

int CBuffer::Read(char* res, int len) {
	if (!_buffer_read) {
		return 0;
	}

	std::unique_lock<std::mutex> lock(_mutex);
	CLoopBuffer* temp = _buffer_read;
	int cur_len = 0;
	while (temp) {
		cur_len += temp->Read(res, len - cur_len);
		if (cur_len >= len) {
			break;
		}
		if (temp == _buffer_write) {
			_pool->PoolDelete<CLoopBuffer>(temp);
			_Reset();
			break;
		}
		_pool->PoolDelete<CLoopBuffer>(temp);
		temp = temp->GetNext();
	}
	_buffer_read = temp;
	return cur_len;
}

int CBuffer::Write(const char* str, int len) {
	int size = 0, current_write_bytes = 0;
	int left = len;

	std::unique_lock<std::mutex> lock(_mutex);
	CLoopBuffer* prv_temp = nullptr;
	CLoopBuffer* temp = _buffer_write;
	int cur_len = 0;
	while (cur_len < len) {
		if (prv_temp != nullptr) {
			prv_temp->SetNext(temp);
		}
		if (temp == nullptr) {
			temp = _pool->PoolNew<CLoopBuffer>(_pool);
		}
		cur_len += temp->Write(str + cur_len, len - cur_len);
		prv_temp = temp;
		temp = temp->GetNext();
	}
	_buffer_write = temp;
	_buffer_end = _buffer_write;
	return cur_len;
}

void CBuffer::Clear(int len) {
	if (len == 0) {
		std::unique_lock<std::mutex> lock(_mutex);
	    CLoopBuffer* temp = _buffer_read;
	    CLoopBuffer* cur = nullptr;
	    while (temp) {
		    cur = temp;
		    temp = temp->GetNext();
		    _pool->PoolDelete<CLoopBuffer>(cur);
	    }
	    _Reset();
		return;
	}
	
	std::unique_lock<std::mutex> lock(_mutex);
	CLoopBuffer* temp = _buffer_read;
	int cur_len = 0;
	while (temp) {
		cur_len += temp->Clear(len - cur_len);
		if (cur_len >= len) {
			break;
		}
		if (temp == _buffer_write) {
			_pool->PoolDelete<CLoopBuffer>(temp);
			_Reset();
			break;
		}
		_pool->PoolDelete<CLoopBuffer>(temp);
		temp = temp->GetNext();
	}
	_buffer_read = temp;
}

int CBuffer::ReadUntil(char* res, int len) {
	if (GetCanReadLength() < len) {
		return 0;

	} else {
		return Read(res, len);
	}
}

int CBuffer::ReadUntil(char* res, int len, const char* find, int find_len, int& need_len) {
	int size = FindStr(find, find_len);
	if (size) {
		if (size <= len) {
			return Read(res, size);

		} else {
			need_len = size;
			return 0;
		}
	}
	return 0;
}

int CBuffer::GetFreeLength() {
	if (!_buffer_write) {
		return 0;
	}
	
	std::unique_lock<std::mutex> lock(_mutex);
	CLoopBuffer* temp = _buffer_write;
	int cur_len = 0;
	while (temp) {
		cur_len += temp->GetCanReadLength();
		if (temp == _buffer_end) {
			break;
		}
		temp = temp->GetNext();
	}
	return cur_len;
}

int CBuffer::GetCanReadLength() {
	if (!_buffer_read) {
		return 0;
	}

    std::unique_lock<std::mutex> lock(_mutex);
	CLoopBuffer* temp = _buffer_read;
	int cur_len = 0;
	while (temp) {
		cur_len += temp->GetCanReadLength();
		if (temp == _buffer_write) {
			break;
		}
		temp = temp->GetNext();
	}
	return cur_len;
}

bool CBuffer::GetFreeMemoryBlock(std::vector<std::pair<void*, int>>& block_vec, int size) {
	std::unique_lock<std::mutex> lock(_mutex);

	void* mem_1 = nullptr;
	void* mem_2 = nullptr;
	int mem_len_1 = 0;
	int mem_len_2 = 0;

	CLoopBuffer* temp = _buffer_write;
	CLoopBuffer* prv_temp = nullptr;
	int cur_len = 0;

	while (cur_len < size) {
		if (prv_temp != nullptr) {
			prv_temp->SetNext(temp);
		}
		if (temp == nullptr) {
			temp = _pool->PoolNew<CLoopBuffer>(_pool);
		}
		
		temp->GetFreeMemoryBlock(mem_1, mem_len_1, mem_2, mem_len_2);
		if (mem_len_1 > 0) {
			block_vec.push_back(std::make_pair(mem_1, mem_len_1));
			cur_len += mem_len_1;
		}
		if (mem_len_2 > 0) {
			block_vec.push_back(std::make_pair(mem_2, mem_len_2));
			cur_len += mem_len_2;
		}
		prv_temp = temp;
		temp = temp->GetNext();
	}
	_buffer_end = temp;
	return cur_len;
}

int CBuffer::FindStr(const char* s, int s_len) const {
	if (!_buffer_read) {
		return 0;
	}

	CLoopBuffer* temp = _buffer_read;
	int cur_len = 0;
	int can_read_bytes = 0;
	bool find = false;
	while (temp) {
		can_read_bytes = temp->FindStr(s, s_len);
		if (can_read_bytes > 0) {
			cur_len += can_read_bytes;
			find = true;
			break;
		}
		if (temp == _buffer_write) {
			break;
		}
		cur_len += temp->GetCanReadLength();
		temp = temp->GetNext();
	}
	return cur_len;
}

void CBuffer::_Reset() {
	_buffer_num = 0;
	_buffer_end = nullptr;
	_buffer_read = nullptr;
	_buffer_write = nullptr; 
}

std::ostream& base::operator<< (std::ostream &out, const CBuffer &obj) {
	if (!obj._buffer_read) {
		return out;
	}
	CLoopBuffer* temp = obj._buffer_read;
	while (temp) {
		out << *temp;
		if (temp == obj._buffer_write) {
			break;
		}
		temp = temp->GetNext();
	}
	return out;
}