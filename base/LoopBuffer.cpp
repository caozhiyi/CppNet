#include <algorithm>
#include <iostream>

#include "LoopBuffer.h"
#include "MemoryPool.h"

using namespace base;

CLoopBuffer::CLoopBuffer(std::shared_ptr<CMemoryPool>& pool) : 
    _can_read(false), 
    _next(nullptr), 
    _pool(pool) {
    
    _buffer_start = pool->PoolLargeMalloc<char>();
    _total_size = pool->GetLargeBlockLength();
    _buffer_end = _buffer_start + _total_size;
    _read = _write = _buffer_start;
}

CLoopBuffer::~CLoopBuffer() {
    if (_buffer_start) {
        _pool->PoolLargeFree<char>(_buffer_start);
    }
}

int CLoopBuffer::ReadNotClear(char* res, int len) {
    return _Read(res, len, false);
}

int CLoopBuffer::Read(char* res, int len) {
    if (res == nullptr) {
        return 0;
    }
    return _Read(res, len, true);
}

int CLoopBuffer::Write(const char* str, int len) {
    if (str == nullptr) {
        return 0;
    }
    return _Write(str, len, true);
}

int CLoopBuffer::Clear(int len) {
    if (len == 0) {
        std::unique_lock<std::mutex> lock(_mutex);
        _write = _read = _buffer_start;
        _can_read = false;
        return 0;
    }

    std::unique_lock<std::mutex> lock(_mutex);
    if (!_buffer_start) {
        return 0;
    }

    if (_read < _write) {
        auto size = _write - _read;
        // res can load all
        if (size <= len) {
            _write = _read = _buffer_start;
            _can_read = false;
            return (int)size;

        // only read len
        } else {
            _read += len;
            return len;
        }

    } else {
        if(!_can_read && _read == _write) {
            return 0;
        }
        auto size_start = _write - _buffer_start;
        auto size_end = _buffer_end - _read;
        auto size =  size_start + size_end;
        // res can load all
        if (size <= len) {
            _read = _write = _buffer_start;
            _can_read = false;
            return (int)size;

        } else {
            if (len <= size_end) {
                _read += len;
                return len;

            } else {
                auto left = len - (size_end);
                _read = _buffer_start + left;
                return len;
            }
        }
    } 
}

int CLoopBuffer::MoveWritePt(int len) {
    return _Write(nullptr, len, false);
}

int CLoopBuffer::ReadUntil(char* res, int len) {
    if (GetCanReadLength() < len) {
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

int CLoopBuffer::GetFreeLength() {
    std::unique_lock<std::mutex> lock(_mutex);
    if (_write > _read) {
        return (int)((_buffer_end - _write) + (_read - _buffer_start));
    
    } else if (_write < _read) {
        return (int)((_read - _write));

    } else {
        if (_can_read) {
            return 0;
        
        } else {
            return _total_size;
        }
    }
}

int CLoopBuffer::GetCanReadLength() {
    std::unique_lock<std::mutex> lock(_mutex);
    if (_write > _read) {
        return (int)(_write - _read);

    } else if (_write < _read) {
        return (int)((_buffer_end - _read) + (_write - _buffer_start));

    } else {
        if (_can_read) {
            return _total_size;

        } else {
            return 0;
        }
    }
}

bool CLoopBuffer::GetFreeMemoryBlock(void*& res1, int& len1, void*& res2, int& len2) {
    res1 = res2 = nullptr;
    len1 = len2 = 0;

    std::unique_lock<std::mutex> lock(_mutex);
    if (_write >= _read) {
        if (_can_read && _write == _read) {
            return false;
        }
        res1 = _write;
        len1 = (int)(_buffer_end - _write);

        len2 = (int)(_read - _buffer_start);
        if(len2 > 0) {
            res2 = _buffer_start;
        }
        return true;

    } else {
        res1 = _write;
        len1 = (int)(_read - _write);
        return true;
    }
}

bool CLoopBuffer::GetUseMemoryBlock(void*& res1, int& len1, void*& res2, int& len2) {
    res1 = res2 = nullptr;
    len1 = len2 = 0;

    std::unique_lock<std::mutex> lock(_mutex);
    if (_read >= _write) {
        if (!_can_read && _write == _read) {
            return false;
        }
        res1 = _read;
        len1 = (int)(_buffer_end - _read);

        len2 = (int)(_write - _buffer_start);
        if(len2 > 0) {
            res2 = _buffer_start;
        }
        return true;

    } else {
        res1 = _read;
        len1 = (int)(_write - _read);
        return true;
    }
}

int CLoopBuffer::FindStr(const char* s, int s_len) {
    std::unique_lock<std::mutex> lock(_mutex);
    if (_write > _read) {
        const char* find = _FindStrInMem(_read, s, _write - _read, s_len);
        if (find) {
            return (int)(find - _read + s_len);
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

CLoopBuffer* CLoopBuffer::GetNext() {
    std::unique_lock<std::mutex> lock(_mutex);
    return _next;
}

void CLoopBuffer::SetNext(CLoopBuffer* next) {
    std::unique_lock<std::mutex> lock(_mutex);
    _next = next;
}

const char* CLoopBuffer::_FindStrInMem(const char* buffer, const char* ch, int buffer_len, int ch_len) const {
    if (!buffer) {
        return nullptr;
    }

    const char* buff = buffer;
    const char* find = nullptr;
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

int CLoopBuffer::_Read(char* res, int len, bool clear) {
    std::unique_lock<std::mutex> lock(_mutex);
    if (!_buffer_start) {
        return 0;
    }
    if (_read < _write) {
        auto size = _write - _read;
        // res can load all
        if (size <= len) {
            memcpy(res, _read, size);
            if(clear) {
                // reset point
                _write = _read = _buffer_start;
                _can_read = false;
            }
            return (int)size;

        // only read len
        } else {
            memcpy(res, _read, len);
            if(clear) {
                _read += len;
            }
            return len;
        }

    } else {
        if(!_can_read && _read == _write) {
            return 0;
        }
        auto size_start = _write - _buffer_start;
        auto size_end = _buffer_end - _read;
        auto size =  size_start + size_end;
        // res can load all
        if (size <= len) {
            memcpy(res, _read, size_end);
            memcpy(res + size_end, _buffer_start, size_start);
            if(clear) {
                // reset point
                _read = _write = _buffer_start;
                _can_read = false;
            }
            return (int)size;

        } else {
            if (len <= size_end) {
                memcpy(res, _read, len);
                if(clear) {
                    _read += len;
                }
                return len;

            } else {
                auto left = len - (size_end);
                memcpy(res, _read, size_end);
                memcpy(res + size_end, _buffer_start, left);
                if(clear) {
                    _read = _buffer_start + left;
                }
                return len;
            }
        }
    } 
}

int CLoopBuffer::_Write(const char* str, int len, bool write) {
    std::unique_lock<std::mutex> lock(_mutex);
    if (_read < _write) {
        if (_write + len <= _buffer_end) {
            if (write) {
                memcpy(_write, str, len);
            }
            _write += len;
            return len;
        
        } else {
            auto size_end = _buffer_end - _write;
            auto left = len - size_end;
            if (write) {
                memcpy(_write, str, size_end);
            }

            if (_buffer_start + left <= _read) {
                if (write) {
                    memcpy(_buffer_start, str + size_end, left);
                }
                _write = _buffer_start + left;
                if (_write == _read) {
                    _can_read = true;
                }
                return len;

            } else {
                auto can_save = _read - _buffer_start;
                if (can_save > 0 && write) {
                    memcpy(_buffer_start, str + size_end, can_save);
                }
                _read = _write;
                _can_read = true;
                return  (int)(can_save + size_end);
            }
        }
    
    } else if (_read > _write) {
        auto size = _read - _write;
        if (len <= size) {
            if (write) {
                memcpy(_write, str, len);
            }
            _write += len;
            if (_write == _read) {
                _can_read = true;
            }
            return len;

        } else {
            if (write) {
                memcpy(_write, str, size);
            }
            _write += size;
            _can_read = true;

            return (int)size;
        }
    
    } else {
        // there is no free memory
        if (_can_read) {
            return 0;

        } else {
            // reset
            _read = _write = _buffer_start;
            int size = _total_size;
            if (size <= len) {
                if (write) {
                    memcpy(_write, str, size);
                }
                _can_read = true;
                return size;

            } else {
                if (write) {
                    memcpy(_write, str, len);
                }
                _write += len;
                return len;
            }
        }
    }
}

std::ostream& base::operator<< (std::ostream &out, const base::CLoopBuffer &obj) {
    if (obj._read < obj._write) {
        out.write(obj._read, obj._write - obj._read);
    } else if (obj._read >= obj._write) {
        out.write(obj._read, obj._buffer_end - obj._read);
        out.write(obj._buffer_start, obj._write - obj._buffer_start);
    } 
    return out;
}
