#include <algorithm>
#include <iostream>

#include "LoopBuffer.h"
#include "MemaryPool.h"


CLoopBuffer::CLoopBuffer(std::shared_ptr<CMemoryPool>& pool, int index) : 
	_can_read(false), 
	_next(nullptr),
	_pool(pool),
	_index(index),
	_buffer_start(nullptr), 
	_total_size(0),
	_buffer_end(nullptr),
	_read (nullptr),
	_write(nullptr) {
	
}

CLoopBuffer::CLoopBuffer(std::shared_ptr<CMemoryPool>& pool, int size, int index) : 
	_can_read(false), 
	_next(nullptr), 
	_pool(pool),
	_index(index),
	_buffer_start(nullptr),
	_total_size(size),
	_buffer_end(nullptr),
	_read(nullptr),
	_write(nullptr) {

}

CLoopBuffer::~CLoopBuffer() {
	if (_buffer_start) {
		_pool->PoolLargeFree<char>(_buffer_start, _total_size);
	}
}

int CLoopBuffer::ReadNotClear(char* res, int len) {
	std::unique_lock<std::mutex> lock(_mutex);
	if (!_buffer_start) {
		return 0;
	}
	if (_read < _write) {
		int size = _write - _read;
		if (size <= len) {
			memcpy(res, _read, size);
			_write = _read = _buffer_start;
			_can_read = false;
			return size;

		} else {
			memcpy(res, _read, len);
			return len;
		}

	} else if (_read > _write) {
		int size = _write - _buffer_start + _buffer_end - _read;
		if (size <= len) {
			memcpy(res, _read, _buffer_end - _read);
			memcpy(res + (_buffer_end - _read), _buffer_start, _write - _buffer_start);
			_can_read = false;
			return size;

		} else {
			if (_read + len <= _buffer_end) {
				memcpy(res, _read, len);
				return len;

			}
			else {
				int left = len - (_buffer_end - _read);
				memcpy(res, _read, len - left);
				memcpy(res + (len - left), _buffer_start, left);
				return len;
			}
		}

	} else {
		if (_can_read) {
			int size = _total_size;
			if (size <= len) {
				memcpy(res, _read, _buffer_end - _read);
				memcpy(res + (_buffer_end - _read), _buffer_start, _read - _buffer_start);
				return size;

			} else {
				if (_read + len > _buffer_end) {
					int left = len - (_buffer_end - _read);
					memcpy(res, _read, len - left);
					memcpy(res + len - left, _buffer_start, left);
					return len;
				} else {
					memcpy(res, _read, len);
					return len;
				}
			}

		} else {
			return 0;
		}
	}
}

int CLoopBuffer::Read(char* res, int len) {
	std::unique_lock<std::mutex> lock(_mutex);
	if (!_buffer_start) {
		return 0;
	}
	if (_read < _write) {
		int size = _write - _read;
		if (size <= len) {
			memcpy(res, _read, size);
			_read += size;
			_write = _read = _buffer_start;
			_can_read = false;

			return size;
		} else {
			memcpy(res, _read, len);
			_read += len;
			return len;
		}

	} else if (_read > _write) {
		int size = _write - _buffer_start + _buffer_end - _read;
		if (size <= len) {
			memcpy(res, _read, _buffer_end - _read);
			memcpy(res + (_buffer_end - _read), _buffer_start, _write - _buffer_start);
			_read = _write = _buffer_start;
			_can_read = false;
			return size;

		} else {
			if (_read + len <= _buffer_end) {
				memcpy(res, _read, len);
				_read += len;
				return len;

			} else {
				int left = len - (_buffer_end - _read);
				memcpy(res, _read, len - left);
				memcpy(res + (len - left), _buffer_start, left);
				_read = _buffer_start + left;
				return len;
			}
		}

	} else {
		if (_can_read) {
			int size = _total_size;
			if (size <= len) {
				memcpy(res, _read, _buffer_end - _read);
				memcpy(res + (_buffer_end - _read), _buffer_start, _read - _buffer_start);
				_read = _write = _buffer_start;
				_can_read = false;
				return size;

			} else {
				if (_read + len > _buffer_end) {
					int left = len - (_buffer_end - _read);
					memcpy(res, _read, len - left);
					memcpy(res + len - left, _buffer_start, left);
					_read = _buffer_start + left;
					return len;
				} else {
					memcpy(res, _read, len);
					_read = _read + len;
					return len;
				}
			}

		} else {
			return 0;
		}
	}
}

int CLoopBuffer::Write(char* str, int len) {
	std::unique_lock<std::mutex> lock(_mutex);
	if (_read < _write) {
		if (_write + len <= _buffer_end) {
			memcpy(_write, str, len);
			_write += len;
			return len;
		
		} else {
			int left = len - (_buffer_end - _write);
			memcpy(_write, str, len - left);
			if (_buffer_start + left <= _read) {
				memcpy(_buffer_start, str + len - left, left);
				_write = _buffer_start + left;
				if (_buffer_start + left == _read)
					_can_read = true;
				
				return len;

			} else {
				int can_save = _read - _buffer_start;
				if (can_save) {
					memcpy(_buffer_start, str + len - left, can_save);
				}
				_write = _read;
				_can_read = true;
				return  len - left + can_save;
			}
		}
	
	} else if (_read > _write) {
		int size = _read - _write;
		if (len <= size) {
			memcpy(_write, str, len);
			_write += len;
			_can_read = true;
			return len;

		} else {
			memcpy(_write, str, size);
			_write += size;
			_can_read = true;

			return size;
		}
	
	} else {
		if (_can_read) {
			return 0;
		} else {
			//malloc memory from pool
			if (!_buffer_start) {
				if (_total_size) {
					_buffer_start = _pool->PoolLargeMalloc<char>(_total_size, _total_size);
					_buffer_end = _buffer_start + _total_size;
					_read = _write = _buffer_start;
				
				} else {
					_buffer_start = _pool->PoolLargeMalloc<char>();
					_total_size = _pool->GetLargeSize();
					_buffer_end = _buffer_start + _total_size;
					_read = _write = _buffer_start;
				}
			}

			int size = _total_size;
			if (size <= len) {
				memcpy(_write, str, size);
				_can_read = true;
				return size;
			} else {
				memcpy(_write, str, len);
				_write += len;
				return len;
			}
		}
	}
}

void CLoopBuffer::Clear() {
	std::unique_lock<std::mutex> lock(_mutex);
	_read = _buffer_start;
	_write = _buffer_start;
	_can_read = false;
}

int CLoopBuffer::Clear(int len) {
	std::unique_lock<std::mutex> lock(_mutex);
	if (!_buffer_start) {
		return 0;
	}
	if (_read < _write) {
		int size = _write - _read;
		if (size <= len) {
			_read += size;
			_write = _read = _buffer_start;
			_can_read = false;

			return size;
		} else {
			_read += len;
			return len;
		}

	} else if (_read > _write) {
		int size = _write - _buffer_start + _buffer_end - _read;
		if (size <= len) {
			_read = _write = _buffer_start;
			_can_read = false;
			return size;

		} else {
			if (_read + len <= _buffer_end) {
				_read += len;
				return len;

			} else {
				int left = len - (_buffer_end - _read);
				_read = _buffer_start + left;
				return len;
			}
		}

	} else {
		if (_can_read) {
			int size = _total_size;
			if (size <= len) {
				_read = _write = _buffer_start;
				_can_read = false;
				return size;

			} else {
				if (_read + len > _buffer_end) {
					int left = len - (_buffer_end - _read);
					_read = _buffer_start + left;
					return len;
				} else {
					_read = _read + len;
					return len;
				}
			}

		} else {
			return 0;
		}
	}
}

int CLoopBuffer::ReadUntil(char* res, int len) {
	if (GetCanReadSize() < len) {
		return 0;
	
	} else {
		return Read(res, len);
	}
}

int CLoopBuffer::ReadUntil(char* res, int len, const char* find, int find_len, int& need_len) {
	int size = FindStr(find, find_len);
	if (size) {
		if (size <= len) {
			return Read(res, len);
		
		} else {
			need_len = size;
			return 0;
		}
	}

	return 0;
}

int CLoopBuffer::GetFreeSize() const {
	if (_write > _read) {
		return (_buffer_end - _write) + (_read - _buffer_start);
	
	} else if (_write < _read) {
		return (_read - _write);

	} else {
		if (_can_read) {
			return 0;
		
		} else {
			return _total_size;
		}
	}
}

int CLoopBuffer::GetCanReadSize() const {
	if (_write > _read) {
		return (_write - _read);

	} else if (_write < _read) {
		return (_buffer_end - _read) + (_write - _buffer_start);

	} else {
		if (_can_read) {
			return _total_size;

		} else {
			return 0;
		}
	}
}

CLoopBuffer* CLoopBuffer::GetNext() const {
	return _next;
}

void CLoopBuffer::SetNext(CLoopBuffer* next) {
	_next = next;
}

int CLoopBuffer::GetIndex() const {
	return _index;
}

int CLoopBuffer::FindStr(const char* s, int s_len) const {
	if (_write > _read) {
		const char* find = _FindStrInMem(_read, s, _write - _read, s_len);
		if (find) {
			return find - _read + s_len;
		}
		return 0;
		
	} else if (_write < _read) {
		const char* find = _FindStrInMem(_read, s, _buffer_end - _read, s_len);
		if (find) {
			return find - _read + s_len;
		}
		find = _FindStrInMem(_buffer_start, s, _write - _buffer_start, s_len);
		if (find) {
			return find - _buffer_start + s_len + _buffer_end - _read;
		}
		return 0;

	} else {
		if (_can_read) {
			const char* find = _FindStrInMem(_read, s, _buffer_end - _read, s_len);
			if (find) {
				return find - _read + s_len;
			}
			find = _FindStrInMem(_buffer_start, s, _write - _buffer_start, s_len);
			if (find) {
				return find - _buffer_start + s_len + _buffer_end - _read;
			}
			return 0;

		} else {
			return 0;
		}
	}
}

void CLoopBuffer::IncrefIndex(int step) {
	_index += step;
}

void CLoopBuffer::DecrefIndex(int step) {
	_index -= step;
}

bool CLoopBuffer::CheckUnused() const {
	if (_read == _write && !_can_read){
		return true;
	}
	return false;
}

const char* CLoopBuffer::_FindStrInMem(const char* buffer, const char* ch, int buffer_len, int ch_len) const {
	if (!buffer) {
		return nullptr;
	}

	const char* buff = buffer;
	const char* find = nullptr;
	int sz = 0;
	int finded = 0;
	while(true) {
		find = (char*)memchr(buff, *ch, buffer_len - finded);
		if (!find) {
			break;
		}
		if (memcmp(find, ch, ch_len) == 0) {
			return find;
		}
		finded += find - buff + 1;
		if (buffer_len - finded < ch_len) {
			break;
		}
		buff = ++find;
	}
	return nullptr;
}

bool operator<(const CLoopBuffer& buf1, const CLoopBuffer& buf2) {
	return buf1._index < buf2._index;
}

bool operator>(const CLoopBuffer& buf1, const CLoopBuffer& buf2) {
	return buf1._index > buf2._index;
}

bool operator<=(const CLoopBuffer& buf1, const CLoopBuffer& buf2) {
	return buf1._index <= buf2._index;
}

bool operator>=(const CLoopBuffer& buf1, const CLoopBuffer& buf2) {
	return buf1._index >= buf2._index;
}

bool operator==(const CLoopBuffer& buf1, const CLoopBuffer& buf2) {
	return buf1._index == buf2._index;
}

bool operator!=(const CLoopBuffer& buf1, const CLoopBuffer& buf2) {
	return buf1._index != buf2._index;
}

std::ostream & operator<< (std::ostream &out, const CLoopBuffer &obj) {
	if (obj._read < obj._write) {
		out.write(obj._read, obj._write - obj._read);
	} else if (obj._read >= obj._write) {
		out.write(obj._read, obj._buffer_end - obj._read);
		out.write(obj._buffer_start, obj._write - obj._buffer_start);
	} 
	return out;
}