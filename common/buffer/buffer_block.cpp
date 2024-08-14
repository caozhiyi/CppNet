// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#include <cstring>
#include "common/alloter/pool_block.h"
#include "common/buffer/buffer_block.h"

namespace cppnet {

BufferBlock::BufferBlock(std::shared_ptr<BlockMemoryPool>& alloter) : 
    _can_read(false),
    _alloter(alloter) {

    _buffer_start = (char*)alloter->PoolLargeMalloc();
    _total_size = alloter->GetBlockLength();
    _buffer_end = _buffer_start + _total_size;
    _read = _write = _buffer_start;
}

BufferBlock::~BufferBlock() {
    if (_buffer_start) {
        auto alloter = _alloter.lock();
        if (alloter) {
            void* m = (void*)_buffer_start;
            alloter->PoolLargeFree(m);
        }
    }
}

uint32_t BufferBlock::ReadNotMovePt(char* res, uint32_t len) {
    return _Read(res, len, false);
}

uint32_t BufferBlock::Read(std::shared_ptr<InnerBuffer> buffer, uint32_t len) {
    if (!_buffer_start) {
        return 0;
    }

    if(!_can_read && _read == _write) {
        return 0;
    }

    if (len == 0) {
        len = GetCanReadLength();
    }

    void* data1 = nullptr, *data2 = nullptr;
    uint32_t len1 = 0, len2 = 0;

    GetUseMemoryBlock(data1, len1, data2, len2);
    uint32_t total_size = len1 + len2;
    uint32_t read_done_size = 0;
    
    // read all data
    if (len >= total_size) {
        read_done_size += buffer->Write((char*)data1, len1);
        read_done_size += buffer->Write((char*)data2, len2);

    // write part of data
    } else if (len < total_size && len >= len1) {
        read_done_size += buffer->Write((char*)data1, len1);
        read_done_size += buffer->Write((char*)data2, len - len1);

    // write part of data
    } else {
        read_done_size += buffer->Write((char*)data1, len);
    }

    MoveReadPt(read_done_size);
    return read_done_size;
}

uint32_t BufferBlock::Write(std::shared_ptr<InnerBuffer> buffer, uint32_t len) {
    if (len == 0) {
        len = buffer->GetCanReadLength();
    }

    if (!_buffer_start) {
        return 0;
    }

    if(!_can_read && _read == _write) {
        _write = _read = _buffer_start;
    }

    void* data1 = nullptr, *data2 = nullptr;
    uint32_t len1 = 0, len2 = 0;

    std::shared_ptr<BufferBlock> block_buffer = std::dynamic_pointer_cast<BufferBlock>(buffer);
    block_buffer->GetUseMemoryBlock(data1, len1, data2, len2);
    uint32_t total_size = len1 + len2;
    uint32_t write_done_size = 0;
    
    // write all data
    if (len >= total_size) {
        write_done_size += Write((char*)data1, len1);
        write_done_size += Write((char*)data2, len2);

    // write part of data
    } else if (len < total_size && len >= len1) {
        write_done_size += Write((char*)data1, len1);
        write_done_size += Write((char*)data2, len - len1);

    // write part of data
    } else {
        write_done_size += Write((char*)data1, len);
    }

    buffer->MoveReadPt(write_done_size);
    return write_done_size;
}

uint32_t BufferBlock::Read(char* res, uint32_t len) {
    if (res == nullptr) {
        return 0;
    }
    return _Read(res, len, true);
}

uint32_t BufferBlock::Write(const char* data, uint32_t len) {
    if (data == nullptr) {
        return 0;
    }
    return _Write(data, len);
}

void BufferBlock::Clear() {
    _write = _read = _buffer_start;
    _can_read = false;
}

int32_t BufferBlock::MoveReadPt(int32_t len) {
    if (!_buffer_start) {
        return 0;
    }

    if (len > 0) {
        /*s-----------r-----w-------------e*/
        if (_read < _write) {
            size_t size = _write - _read;
            // res can load all
            if ((int32_t)size <= len) {
                Clear();
                return (int32_t)size;

            // only read len
            } else {
                _read += len;
                return len;
            }

        /*s-----------w-----r-------------e*/
        /*s-----------wr------------------e*/
        } else {
            if(!_can_read && _read == _write) {
                return 0;
            }
            size_t size_start = _write - _buffer_start;
            size_t size_end = _buffer_end - _read;
            size_t size =  size_start + size_end;
            // res can load all
            if ((int32_t)size <= len) {
                Clear();
                return (int32_t)size;

            // only read len
            } else {
                if (len <= (int32_t)size_end) {
                    _read += len;
                    return len;

                } else {
                    size_t left = len - size_end;
                    _read = _buffer_start + left;
                    return len;
                }
            }
        }

    } else {
        len = -len;
        /*s-----------w-----r-------------e*/
        if (_write < _read) {
            size_t size = _read - _write;
            // reread all buffer
            if ((int32_t)size <= len) {
                _read = _write;
                _can_read = true;
                return (int32_t)size;

            // only reread part of buffer
            } else {
                _read -= len;
                return len;
            }
        
        /*s-----------r-----w-------------e*/
        /*s-----------rw------------------e*/
        } else {
            if(_can_read && _read == _write) {
                return 0;
            }
            size_t size_start = _read - _buffer_start;
            size_t size_end = _buffer_end - _write;
            size_t size =  size_start + size_end;
            // reread all buffer
            if ((int32_t)size <= len) {
                _read = _write;
                _can_read = true;
                return (int32_t)size;

            // only reread part of buffer
            } else {
                if (len <= (int32_t)size_start) {
                    _read -= len;
                    return len;

                } else {
                    size_t left = len - size_start;
                    _read = _buffer_end - left;
                    return len;
                }
            }
        }
    }
}

int32_t BufferBlock::MoveWritePt(int32_t len) {
    if (!_buffer_start) {
        return 0;
    }

    if (len > 0) {
        /*s-----------w-----r-------------e*/
        if (_write < _read) {
            size_t size = _read - _write;
            // all buffer will be used
            if ((int32_t)size <= len) {
                _write = _read;
                _can_read = true;
                return (int32_t)size;

            // part of buffer will be used
            } else {
                _write += len;
                return len;
            }

        /*s-----------r-----w-------------e*/
        /*s-----------rw------------------e*/
        } else {
            if(_can_read && _read == _write) {
                return 0;
            }
            size_t size_start = _read - _buffer_start;
            size_t size_end = _buffer_end - _write;
            size_t size =  size_start + size_end;

            // all buffer will be used
            if ((int32_t)size <= len) {
                _write = _read;
                _can_read = true;
                return (int32_t)size;

            // part of buffer will be used
            } else {
                if (len <= (int32_t)size_end) {
                    _write += len;
                    return len;

                } else {
                    size_t left = len - size_end;
                    _write = _buffer_start + left;
                    return len;
                }
            }
        }

    } else {
        len = -len;
        /*s-----------r-----w-------------e*/
        if (_read < _write) {
            size_t size = _write - _read;
            // rewrite all buffer
            if ((int32_t)size <= len) {
                Clear();
                return (int32_t)size;

            // only rewrite part of buffer
            } else {
                _write -= len;
                return len;
            }
        
        /*s-----------w-----r-------------e*/
        /*s-----------wr------------------e*/
        } else {
            if(!_can_read && _read == _write) {
                return 0;
            }
            size_t size_start = _write - _buffer_start;
            size_t size_end = _buffer_end - _read;
            size_t size =  size_start + size_end;
            // rewrite all buffer
            if ((int32_t)size <= len) {
                Clear();
                return (int32_t)size;

            // only rewrite part of buffer
            } else {
                if (len <= (int32_t)size_start) {
                    _write -= len;
                    return len;

                } else {
                    size_t left = len - size_start;
                    _write = _buffer_end - left;
                    return len;
                }
            }
        }
    }
}

uint32_t BufferBlock::ReadUntil(char* res, uint32_t len) {
    if (GetCanReadLength() < len) {
        return 0;
    
    } else {
        return Read(res, len);
    }
}
    
uint32_t BufferBlock::ReadUntil(char* res, uint32_t len, const char* find, uint32_t find_len, uint32_t& need_len)  {
    uint32_t size = FindStr(find, find_len);
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
    
uint32_t BufferBlock::GetCanWriteLength() {
    if (_write > _read) {
        return (uint32_t)((_buffer_end - _write) + (_read - _buffer_start));
    
    } else if (_write < _read) {
        return (uint32_t)((_read - _write));

    } else {
        if (_can_read) {
            return 0;
        
        } else {
            return _total_size;
        }
    }
}

uint32_t BufferBlock::GetCanReadLength() {
    if (_write > _read) {
        return (uint32_t)(_write - _read);

    } else if (_write < _read) {
        return (uint32_t)((_buffer_end - _read) + (_write - _buffer_start));

    } else {
        if (_can_read) {
            return _total_size;

        } else {
            return 0;
        }
    }
}

bool BufferBlock::GetFreeMemoryBlock(void*& res1, uint32_t& len1, void*& res2, uint32_t& len2) {
    res1 = res2 = nullptr;
    len1 = len2 = 0;

    if (_write >= _read) {
        if (_can_read && _write == _read) {
            return false;
        }
        res1 = _write;
        len1 = (uint32_t)(_buffer_end - _write);

        len2 = (uint32_t)(_read - _buffer_start);
        if(len2 > 0) {
            res2 = _buffer_start;
        }
        return true;

    } else {
        res1 = _write;
        len1 = (uint32_t)(_read - _write);
        return true;
    }
}

bool BufferBlock::GetUseMemoryBlock(void*& res1, uint32_t& len1, void*& res2, uint32_t& len2) {
    res1 = res2 = nullptr;
    len1 = len2 = 0;

    if (_read >= _write) {
        if (!_can_read && _write == _read) {
            return false;
        }
        res1 = _read;
        len1 = (uint32_t)(_buffer_end - _read);

        len2 = (uint32_t)(_write - _buffer_start);
        if(len2 > 0) {
            res2 = _buffer_start;
        }
        return true;

    } else {
        res1 = _read;
        len1 = (uint32_t)(_write - _read);
        return true;
    }
}

uint32_t BufferBlock::FindStr(const char* s, uint32_t s_len) {
    if (_write > _read) {
        const char* find = _FindStrInMem(_read, s, uint32_t(_write - _read), s_len);
        if (find) {
            return (uint32_t)(find - _read + s_len);
        }
        return 0;
        
    } else if (_write < _read) {
        const char* find = _FindStrInMem(_read, s, uint32_t(_buffer_end - _read), s_len);
        if (find) {
            return uint32_t(find - _read + s_len);
        }
        find = _FindStrInMem(_buffer_start, s, uint32_t(_write - _buffer_start), s_len);
        if (find) {
            return uint32_t(find - _buffer_start + s_len + _buffer_end - _read);
        }
        return 0;

    } else {
        if (_can_read) {
            const char* find = _FindStrInMem(_read, s, uint32_t(_buffer_end - _read), s_len);
            if (find) {
                return uint32_t(find - _read + s_len);
            }
            find = _FindStrInMem(_buffer_start, s, uint32_t(_write - _buffer_start), s_len);
            if (find) {
                return uint32_t(find - _buffer_start + s_len + _buffer_end - _read);
            }
            return 0;

        } else {
            return 0;
        }
    }
}

std::shared_ptr<BlockMemoryPool> BufferBlock::GetBlockMemoryPool() {
    return _alloter.lock();
}

const char* BufferBlock::_FindStrInMem(const char* buffer, const char* ch, uint32_t buffer_len, uint32_t ch_len) const {
    if (!buffer) {
        return nullptr;
    }

    const char* buffer_end = buffer + buffer_len;
    const char* buff = buffer;
    const char* find = nullptr;
    size_t finded = 0;
    while(true) {
        find = (char*)memchr(buff, *ch, buffer_len - finded);
        if (!find || find > buffer_end - ch_len) {
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
    
uint32_t BufferBlock::_Read(char* res, uint32_t len, bool move_pt) {
    /*s-----------r-----w-------------e*/
    if (_read < _write) {
        size_t size = _write - _read;
        // res can load all
        if (size <= len) {
            memcpy(res, _read, size);
            if(move_pt) {
                Clear();
            }
            return (uint32_t)size;

        // only read len
        } else {
            memcpy(res, _read, len);
            if(move_pt) {
                _read += len;
            }
            return len;
        }

    /*s-----------w-----r-------------e*/
    /*s----------------wr-------------e*/
    } else {
        if(!_can_read && _read == _write) {
            return 0;
        }
        size_t size_start = _write - _buffer_start;
        size_t size_end = _buffer_end - _read;
        size_t size =  size_start + size_end;
        // res can load all
        if (size <= len) {
            memcpy(res, _read, size_end);
            memcpy(res + size_end, _buffer_start, size_start);
            if(move_pt) {
                // reset point
                Clear();
            }
            return (uint32_t)size;

        } else {
            if (len <= size_end) {
                memcpy(res, _read, len);
                if(move_pt) {
                    _read += len;
                }
                return len;

            } else {
                size_t left = len - size_end;
                memcpy(res, _read, size_end);
                memcpy(res + size_end, _buffer_start, left);
                if(move_pt) {
                    _read = _buffer_start + left;
                }
                return len;
            }
        }
    }
}
    
uint32_t BufferBlock::_Write(const char* data, uint32_t len) {
    /*s-----------w-----r-------------e*/
    if (_write < _read) {
        size_t size = _read - _write;
        // can save all data
        if (len <= size) {
            memcpy(_write, data, len);
            _write += len;
            if (_write == _read) {
                _can_read = true;
            }
            return len;

        // can save a part of data
        } else {
            memcpy(_write, data, size);
            _write += size;
            _can_read = true;

            return (uint32_t)size;
        }
    
    /*s-----------r-----w-------------e*/
    /*s-----------rw------------------e*/
    } else {
        if(_can_read && _read == _write) {
            return 0;
        }

        size_t size_start = _read - _buffer_start;
        size_t size_end = _buffer_end - _write;
        size_t size =  size_start + size_end;

        // all buffer will be used
        if (size <= len) {
            memcpy(_write, data, size_end);
            memcpy(_buffer_start, data + size_end, size_start);

            _write = _read;
            _can_read = true;
            return (int32_t)size;

        // part of buffer will be used
        } else {
            if (len <= size_end) {
                memcpy(_write, data, len);
                _write += len;
                return len;

            } else {
                size_t left = len - size_end;

                memcpy(_write, data, size_end);
                memcpy(_buffer_start, data + size_end, left);
               
                _write = _buffer_start + left;
                return len;
            }
        }
    }
}

}