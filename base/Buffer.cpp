#include <iostream>
#include "Buffer.h"
#include "MemaryPool.h"
#include "LoopBuffer.h"
#include "Log.h"

CBuffer::CBuffer(std::shared_ptr<CMemoryPool>& pool) : 
	_pool(pool), 
	_buffer_num(0),
	_buffer_start(nullptr),
	_buffer_read(nullptr),
	_buffer_write(nullptr) {

}

CBuffer::~CBuffer() {
	CLoopBuffer* temp = _buffer_start;
	while (temp) {
		_buffer_start = _buffer_start->GetNext();
		_pool->PoolDelete<CLoopBuffer>(temp);
		temp = _buffer_start;
	}
}

int CBuffer::ReadNotClear(char* res, int len) {
	if (!_buffer_read) {
		return 0;
	}

	std::unique_lock<std::mutex> lock(_mutex);
	int size = 0, current_read_bytes = 0;
	int left = len;
	if (*_buffer_read < *_buffer_write) {
		CLoopBuffer* temp = _buffer_read;
		while (temp && *temp != *_buffer_write) {
			current_read_bytes = temp->ReadNotClear(res + size, left);
			left -= current_read_bytes;
			size += current_read_bytes;
			if (size == len) {
				return size;
			}
			temp = temp->GetNext();
		}
		size += temp->ReadNotClear(res + size, left);

	} else if (*_buffer_read >= *_buffer_write) {
		CLoopBuffer* temp = _buffer_read;
		while (temp && *temp != *_buffer_end) {
			current_read_bytes = temp->ReadNotClear(res + size, left);
			left -= current_read_bytes;
			size += current_read_bytes;
			if (size == len) {
				return size;
			}
			temp = temp->GetNext();
		}

		if (temp) {
			current_read_bytes = temp->ReadNotClear(res + size, left);
			left -= current_read_bytes;
			size += current_read_bytes;
			if (size == len) {
				return size;
			}
		}

		temp = _buffer_start;
		while (temp && *temp != *_buffer_write) {
			current_read_bytes = temp->ReadNotClear(res + size, left);
			left -= current_read_bytes;
			size += current_read_bytes;
			if (size == len) {
				return size;
			}
			temp = temp->GetNext();
		}
		size += temp->ReadNotClear(res + size, left);
	}
	return size;
}

int CBuffer::Read(char* res, int len) {
	if (!_buffer_read) {
		return 0;
	}

	std::unique_lock<std::mutex> lock(_mutex);
	if (_buffer_num > __max_node_size) {
		ReleaseUnuseBuffer();
	}

	int size = 0, current_read_bytes = 0;
	int left = len;
	if (*_buffer_read < *_buffer_write) {
		CLoopBuffer* temp = _buffer_read;
		while (temp && *temp != *_buffer_write) {
			current_read_bytes = temp->Read(res + size, left);
			left -= current_read_bytes;
			size += current_read_bytes;
			if (size == len) {
				_buffer_read = temp;
				return size;
			}
			temp = temp->GetNext();
		}
		size += temp->Read(res + size, left);
		_buffer_read = temp;

	} else if (*_buffer_read >= *_buffer_write) {
		CLoopBuffer* temp = _buffer_read;
		while (temp && *temp != *_buffer_end) {
			current_read_bytes = temp->Read(res + size, left);
			left -= current_read_bytes;
			size += current_read_bytes;
			if (size == len) {
				_buffer_read = temp;
				return size;
			}
			temp = temp->GetNext();
		}

		if (temp) {
			current_read_bytes = temp->Read(res + size, left);
			left -= current_read_bytes;
			size += current_read_bytes;
			if (size == len) {
				_buffer_read = temp;
				return size;
			}
		}

		temp = _buffer_start;
		while (temp && *temp != *_buffer_write) {
			current_read_bytes = temp->Read(res + size, left);
			left -= current_read_bytes;
			size += current_read_bytes;
			if (size == len) {
				_buffer_read = temp;
				return size;
			}
			temp = temp->GetNext();
		}
		size += temp->Read(res + size, left);
		_buffer_read = temp;
	}
	return size;
}

int CBuffer::Write(char* str, int len) {
	int size = 0, current_write_bytes = 0;
	int left = len;

	std::unique_lock<std::mutex> lock(_mutex);
	//first buffer node
	if (!_buffer_write && !_buffer_read) {
		try {
			CLoopBuffer* buffer = _pool->PoolNew<CLoopBuffer>(_pool, left, 1);
			_buffer_start = _buffer_end = buffer;
			_buffer_write = buffer;
			_buffer_read = buffer;
			_buffer_num++;

		} catch (std::exception& e) {
			LOG_FATAL("memory is not enough! %s", e.what());
			abort();
		}
	}

	if (*_buffer_read <= *_buffer_write) {
		CLoopBuffer* temp = _buffer_write;
		while (true) {
			current_write_bytes = temp->Write(str + size, left);
			left -= current_write_bytes;
			size += current_write_bytes;
			if (size == len) {
				_buffer_write = temp;
				return size;
			}
			CLoopBuffer* next = temp->GetNext();
			if (!next || *next == *_buffer_read) {
				break;
			}
			temp = next;
		}
		if (temp) {
			current_write_bytes = temp->Write(str + size, left);
			left -= current_write_bytes;
			size += current_write_bytes;
			if (size == len) {
				_buffer_write = temp;
				return size;
			}
		}
		
		temp = _buffer_start;
		while (true) {
			current_write_bytes = temp->Write(str + size, left);
			left -= current_write_bytes;
			size += current_write_bytes;
			if (size == len) {
				_buffer_write = temp;
				return size;
			}
			CLoopBuffer* next = temp->GetNext();
			if (!next || *next == *_buffer_read) {
				break;
			}
			temp = next;
		}

		if (left > 0) {
			try {
				CLoopBuffer* buffer = _pool->PoolNew<CLoopBuffer>(_pool, left, temp->GetIndex());
				buffer->SetNext(temp->GetNext());
				temp->SetNext(buffer);
				temp = temp->GetNext();
				_IncrefIndex(temp);
				_buffer_num++;

			} catch (std::exception& e) {
				LOG_FATAL("memory is not enough! %s", e.what());
				abort();
			}
		}
		size += temp->Write(str + size, left);
		_buffer_write = temp;
		_buffer_end = temp;

	} else if (*_buffer_read > *_buffer_write) {
		CLoopBuffer* temp = _buffer_write;
		while (true) {
			current_write_bytes = temp->Write(str + size, left);
			left -= current_write_bytes;
			size += current_write_bytes;
			if (size == len) {
				_buffer_write = temp;
				return size;
			}
			CLoopBuffer* next = temp->GetNext();
			if (!next || *next == *_buffer_read) {
				break;
			}
			temp = next;
		}

		if (left > 0) {
			try {
				CLoopBuffer* buffer = _pool->PoolNew<CLoopBuffer>(_pool, left, temp->GetIndex());
				buffer->SetNext(temp->GetNext());
				temp->SetNext(buffer);
				temp = temp->GetNext();
				_IncrefIndex(temp);
				_buffer_num++;

			} catch (std::exception& e) {
				LOG_FATAL("memory is not enough! %s", e.what());
				abort();
			}
		}
		size += temp->Write(str + size, left);
		_buffer_write = temp;
	}
	return size;
}

void CBuffer::Clear() {
	if (!_buffer_start) {
		return;
	}
	std::unique_lock<std::mutex> lock(_mutex);
	CLoopBuffer* temp = _buffer_start;
	while (temp && *temp != *_buffer_write) {
		temp->Clear();
		temp = temp->GetNext();
	}
	temp->Clear();
	_buffer_read = _buffer_start;
	_buffer_write = _buffer_start;
}

void CBuffer::Clear(int len) {
	if (!_buffer_read) {
		return;
	}

	std::unique_lock<std::mutex> lock(_mutex);
	if (_buffer_num > __max_node_size) {
		ReleaseUnuseBuffer();
	}

	int size = 0, current_read_bytes = 0;
	int left = len;
	if (*_buffer_read < *_buffer_write) {
		CLoopBuffer* temp = _buffer_read;
		while (temp && *temp != *_buffer_write) {
			current_read_bytes = temp->Clear(left);
			left -= current_read_bytes;
			size += current_read_bytes;
			if (size == len) {
				_buffer_read = temp;
				return;
			}
			temp = temp->GetNext();
		}
		size += temp->Clear(left);
		_buffer_read = temp;

	} else if (*_buffer_read >= *_buffer_write) {
		CLoopBuffer* temp = _buffer_read;
		while (temp && *temp != *_buffer_end) {
			current_read_bytes = temp->Clear(left);
			left -= current_read_bytes;
			size += current_read_bytes;
			if (size == len) {
				_buffer_read = temp;
				return;
			}
			temp = temp->GetNext();
		}

		if (temp) {
			current_read_bytes = temp->Clear(left);
			left -= current_read_bytes;
			size += current_read_bytes;
			if (size == len) {
				_buffer_read = temp;
				return;
			}
		}

		temp = _buffer_start;
		while (temp && *temp != *_buffer_write) {
			current_read_bytes = temp->Clear(left);
			left -= current_read_bytes;
			size += current_read_bytes;
			if (size == len) {
				_buffer_read = temp;
				return;
			}
			temp = temp->GetNext();
		}
		size += temp->Clear(left);
		_buffer_read = temp;
	}
}

int CBuffer::ReadUntil(char* res, int len) {
	if (GetCanReadSize() < len) {
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

int CBuffer::GetFreeSize() const {
	if (!_buffer_write) {
		return 0;
	}
	
	int res = 0;
	if (*_buffer_write >= *_buffer_read) {
		CLoopBuffer* temp = _buffer_write;
		while (*temp != *_buffer_end) {
			res += temp->GetFreeSize();
			temp = temp->GetNext();
		}
		res += temp->GetFreeSize();

		temp = _buffer_start;
		while (*temp != *_buffer_read) {
			res += temp->GetFreeSize();
			temp = temp->GetNext();
		}
		res += temp->GetFreeSize();
		
	} else if (*_buffer_write < *_buffer_read) {
		CLoopBuffer* temp = _buffer_write;
		while (*temp != *_buffer_read) {
			res += temp->GetFreeSize();
			temp = temp->GetNext();
		}
		res += temp->GetFreeSize();

	}
	return res;
}

int CBuffer::GetCanReadSize() const {
	if (!_buffer_read) {
		return 0;
	}

	int res = 0;
	if (*_buffer_write > *_buffer_read) {
		CLoopBuffer* temp = _buffer_read;
		while (*temp != *_buffer_write) {
			res += temp->GetCanReadSize();
			temp = temp->GetNext();
		}
		res += temp->GetCanReadSize();

	} else if (*_buffer_write <= *_buffer_read) {
		CLoopBuffer* temp = _buffer_read;
		while (*temp != *_buffer_end) {
			res += temp->GetCanReadSize();
			temp = temp->GetNext();
		}
		res += temp->GetCanReadSize();

		temp = _buffer_start;
		while (*temp != *_buffer_write) {
			res += temp->GetCanReadSize();
			temp = temp->GetNext();
		}
		res += temp->GetCanReadSize();
	}
	return res;
}

int CBuffer::FindStr(const char* s, int s_len) const {
	if (!_buffer_read) {
		return 0;
	}

	if (*_buffer_write > *_buffer_read) {
		int res_len = 0;
		int can_read_bytes = 0;
		bool find = false;

		CLoopBuffer* temp = _buffer_read;
		while (*temp != *_buffer_write) {
			can_read_bytes = temp->FindStr(s, s_len);
			if (can_read_bytes) {
				res_len += can_read_bytes;
				find = true;
				break;
			
			} else {
				res_len += temp->GetCanReadSize();
			}
		}
		if (!find) {
			can_read_bytes = temp->FindStr(s, s_len);
			if (can_read_bytes) {
				res_len += can_read_bytes;
				find = true;
			}
		}
		
		if (find) {
			return res_len;
		}
		return 0;

	} else if (_buffer_write < _buffer_read) {
		int res_len = 0;
		int can_read_bytes = 0;
		bool find = false;

		CLoopBuffer* temp = _buffer_read;
		while (*temp != *_buffer_end && !find) {
			can_read_bytes = temp->FindStr(s, s_len);
			if (can_read_bytes) {
				res_len += can_read_bytes;
				find = true;
				break;

			} else {
				res_len += temp->GetCanReadSize();
			}
		}
		if (!find) {
			can_read_bytes = temp->FindStr(s, s_len);
			if (can_read_bytes) {
				res_len += can_read_bytes;
				find = true;
			}
		}

		temp = _buffer_start;
		while (*temp != *_buffer_write && !find) {
			can_read_bytes = temp->FindStr(s, s_len);
			if (can_read_bytes) {
				res_len += can_read_bytes;
				find = true;
				break;

			} else {
				res_len += temp->GetCanReadSize();
			}
		}
		if (!find) {
			can_read_bytes = temp->FindStr(s, s_len);
			if (can_read_bytes) {
				res_len += can_read_bytes;
				find = true;
			}
		}

		if (find) {
			return res_len;
		}
		return 0;

	} else {

		return _buffer_read->FindStr(s, s_len);
	}
}

void CBuffer::ReleaseUnuseBuffer() {
	if (!_buffer_start) {
		return;
	}
	std::unique_lock<std::mutex> lock(_mutex);
	if (*_buffer_read <= *_buffer_write) {
		CLoopBuffer* temp = _buffer_write->GetNext();
		while (temp && temp->CheckUnused() && temp != _buffer_end) {
			_buffer_write->SetNext(temp->GetNext());
			_pool->PoolDelete<CLoopBuffer>(temp);
			_buffer_num--;
			temp = _buffer_write->GetNext();
		}

		temp = _buffer_start->GetNext();
		while (temp && temp->CheckUnused() && *temp < *_buffer_read) {
			_buffer_start->SetNext(temp->GetNext());
			_pool->PoolDelete<CLoopBuffer>(temp);
			_buffer_num--;
			temp = _buffer_start->GetNext();
		}

	} else if (*_buffer_read > *_buffer_write) {
		CLoopBuffer* temp = _buffer_write->GetNext();
		while (temp && temp->CheckUnused() && *temp != *_buffer_read) {
			_buffer_write->SetNext(temp->GetNext());
			_pool->PoolDelete<CLoopBuffer>(temp);
			_buffer_num--;
			temp = _buffer_write->GetNext();
			_buffer_write = temp;
		}
	}
}

std::vector<CLoopBuffer*> CBuffer::GetMaxCatch(int size) {
	std::vector<CLoopBuffer*> res;
	return res;
}

std::vector<CLoopBuffer*> CBuffer::GetReadBuffer() {
	std::vector<CLoopBuffer*> res;
	if (!_buffer_read) {
		return res;
	}
	std::unique_lock<std::mutex> lock(_mutex);
	if (*_buffer_read < *_buffer_write) {
		CLoopBuffer* temp = _buffer_read;
		while (temp && *temp != *_buffer_write) {
			res.push_back(temp);
			temp = temp->GetNext();
		}
		if (temp->GetCanReadSize()) {
			res.push_back(temp);
		}

	} else if (*_buffer_read >= *_buffer_write) {
		CLoopBuffer* temp = _buffer_read;
		while (temp && *temp != *_buffer_end) {
			res.push_back(temp);
			temp = temp->GetNext();
		}

		if (temp->GetCanReadSize()) {
			res.push_back(temp);
		}

		temp = _buffer_start;
		while (temp && *temp != *_buffer_write) {
			res.push_back(temp);
			temp = temp->GetNext();
		}
		if (temp->GetCanReadSize()) {
			res.push_back(temp);
		}
	}
	return res;
}

std::ostream& operator<< (std::ostream &out, const CBuffer &obj) {
	if (!obj._buffer_start) {
		return out;
	}

	if (*obj._buffer_read < *obj._buffer_write) {
		CLoopBuffer* temp = obj._buffer_read;
		while (temp && *temp != *obj._buffer_write) {
			out << *temp;
		}
		if (temp->GetCanReadSize()) {
			out << *temp;
		}

	} else if (*obj._buffer_read >= *obj._buffer_write) {
		CLoopBuffer* temp = obj._buffer_read;
		while (temp && *temp != *obj._buffer_end) {
			out << *temp;
		}

		if (temp->GetCanReadSize()) {
			out << *temp;
		}

		temp = obj._buffer_start;
		while (temp && *temp != *obj._buffer_write) {
			out << *temp;
		}
		if (temp->GetCanReadSize() && *temp != *obj._buffer_start) {
			out << *temp;
		}
	}
	return out;
}

void CBuffer::_IncrefIndex(CLoopBuffer* start) {
	CLoopBuffer* temp = start;
	while (temp) {
		temp->IncrefIndex();
		temp = temp->GetNext();
	}
}

void CBuffer::_DecrefIndex(CLoopBuffer* start) {
	CLoopBuffer* temp = start;
	while (temp) {
		temp->DecrefIndex();
		temp = temp->GetNext();
	}
}