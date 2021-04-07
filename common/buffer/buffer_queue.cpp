// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#include "buffer_queue.h"
#include "buffer_block.h"
#include "common/alloter/pool_block.h"
#include "common/alloter/alloter_interface.h"

namespace cppnet {

static const uint8_t __block_vec_default_size = 8;

BufferQueue::BufferQueue(const std::shared_ptr<BlockMemoryPool>& block_pool, 
    const std::shared_ptr<AlloterWrap>& alloter):
    _can_read_length(0),
    _block_alloter(block_pool),
    _alloter(alloter) {

}

BufferQueue::~BufferQueue() {
    _buffer_list.Clear();
}

uint32_t BufferQueue::ReadNotMovePt(char* res, uint32_t len) {
    if (_buffer_list.Size() == 0 || !res) {
        return 0;
    }

    std::shared_ptr<BufferBlock> temp = _buffer_list.GetHead();
    uint32_t read_len = 0;
    while (temp && read_len < len) {
        read_len += temp->ReadNotMovePt(res + read_len, len - read_len);
        if (temp == _buffer_write) {
            break;
        }
        temp = temp->GetNext();
    }
    return read_len;
}

uint32_t BufferQueue::Read(std::shared_ptr<Buffer> buffer, uint32_t len) {
    if (!buffer) {
        return 0;
    }
    
    if (len == 0) {
        len = GetCanReadLength();
    } 
    if (len == 0) {
        return 0;
    }

    std::shared_ptr<BufferQueue> buffer_queue = std::dynamic_pointer_cast<BufferQueue>(buffer);
    if (!buffer_queue->_buffer_write) {
        buffer_queue->Append();
        buffer_queue->_buffer_write = buffer_queue->_buffer_list.GetTail();
    }

    uint32_t can_write_size = buffer_queue->_buffer_write->GetCanWriteLength();
    uint32_t total_read_len = 0;
    uint32_t cur_read_len = 0;

    auto buffer_read = _buffer_list.GetHead();
    while (buffer_read) {
        cur_read_len = buffer_read->Read(buffer_queue->_buffer_write, can_write_size);
        total_read_len += cur_read_len;

        // current write block is full
        if (cur_read_len >= can_write_size) {
            if (total_read_len >= len) {
                break;
            }

            buffer_queue->Append();
            buffer_queue->_buffer_write = buffer_queue->_buffer_list.GetTail();
            can_write_size = buffer_queue->_buffer_write->GetCanWriteLength();

        // current read block is empty
        } else {
            can_write_size -= cur_read_len;
            if (buffer_read == _buffer_write) {
                if (_buffer_write->GetNext()) {
                    _buffer_write = _buffer_write->GetNext();

                } else {
                    Reset();
                    break;
                }
            }
            _buffer_list.PopFront();
            buffer_read = _buffer_list.GetHead();
        }

        if (total_read_len >= len) {
            break;
        }
    }
    _can_read_length -= total_read_len;
    return total_read_len;
}

uint32_t BufferQueue::Write(std::shared_ptr<Buffer> buffer, uint32_t len) {
    if (!buffer) {
        return 0;
    }
    
    if (len == 0) {
        len = buffer->GetCanReadLength();
    }
    if (len == 0) {
        return 0;
    }

    std::shared_ptr<BufferQueue> buffer_queue = std::dynamic_pointer_cast<BufferQueue>(buffer);
    auto from_list = buffer_queue->_buffer_list;
    auto from_buffer = from_list.GetHead();

    uint32_t should_write_size = from_list.GetHead()->GetCanReadLength();
    uint32_t total_write_len = 0;
    uint32_t cur_write_len = 0;

    while (1) {
        if (!_buffer_write) {
            Append();
            _buffer_write = _buffer_list.GetTail();
        }

        cur_write_len = _buffer_write->Write(from_buffer, should_write_size);
        total_write_len += cur_write_len;
        
        // current read block is empty
        if (cur_write_len >= should_write_size) {
            if (from_buffer == buffer_queue->_buffer_write) {
                if (buffer_queue->_buffer_write->GetNext()) {
                    buffer_queue->_buffer_write = buffer_queue->_buffer_write->GetNext();

                } else {
                    buffer_queue->Reset();
                    break;
                }
            }
            from_list.PopFront();
            from_buffer = from_list.GetHead();
            should_write_size = from_buffer->GetCanReadLength();

        // current write block is full
        } else {
            if (total_write_len >= len) {
                break;
            }
            should_write_size -= cur_write_len;
            _buffer_write = _buffer_write->GetNext();
        }

        if (total_write_len >= len) {
            break;
        }
    }
    _can_read_length += total_write_len;
    return total_write_len;
}

uint32_t BufferQueue::Read(char* res, uint32_t len) {
    if (_buffer_list.Size() == 0 || !res || len == 0) {
        return 0;
    }

    auto buffer_read = _buffer_list.GetHead();
    uint32_t total_read_len = 0;
    while (buffer_read) {
        total_read_len += buffer_read->Read(res + total_read_len, len - total_read_len);
        if (total_read_len >= len) {
            break;
        }
        if (buffer_read == _buffer_write) {
            if (_buffer_write->GetNext()) {
                _buffer_write = _buffer_write->GetNext();

            } else {
                Reset();
                break;
            }
        }
        _buffer_list.PopFront();
        buffer_read = _buffer_list.GetHead();
		if (buffer_read->GetCanReadLength() == 0) {
            break;
		}
    }
    _can_read_length -= total_read_len;
    return total_read_len;
}

uint32_t BufferQueue::Write(const char* str, uint32_t len) {
    if (!str || len == 0) {
        return 0;
    }
    
    uint32_t write_len = 0;

    while (1) {
        if (!_buffer_write) {
            Append();
            _buffer_write = _buffer_list.GetTail();
        }

        write_len += _buffer_write->Write(str + write_len, len - write_len);

        if (write_len >= len) {
            break;
        }
        _buffer_write = _buffer_write->GetNext();
    }
    _can_read_length += write_len;
    return write_len;
}

void BufferQueue::Clear() {
    _can_read_length = 0;
    Reset();
}

int32_t BufferQueue::MoveReadPt(int32_t len) {
    int32_t total_read_len = 0;
    auto buffer_read = _buffer_list.GetHead();

    if (len >= 0) {
        while (buffer_read) {
            total_read_len += buffer_read->MoveReadPt(len - total_read_len);

            if (total_read_len >= len) {
                break;
            }

            if (buffer_read == _buffer_write) {
                if (_buffer_write->GetNext()) {
                    _buffer_write = _buffer_write->GetNext();

                } else {
                    Reset();
                    break;
                }
            }
            _buffer_list.PopFront();
            buffer_read = _buffer_list.GetHead();
        }

    } else {
        total_read_len += buffer_read->MoveReadPt(len);
    }

    _can_read_length -= total_read_len;
    return total_read_len;
}

int32_t BufferQueue::MoveWritePt(int32_t len) {
    int32_t total_write_len = 0;
    if (len >= 0) {
        while (_buffer_write) {
            total_write_len += _buffer_write->MoveWritePt(len - total_write_len);
            if (_buffer_write == _buffer_list.GetTail() || len <= total_write_len) {
                break;
            }
            _buffer_write = _buffer_write->GetNext();
        }

    } else {
        while (_buffer_write) {
            total_write_len += _buffer_write->MoveWritePt(len + total_write_len);
            if (_buffer_write == _buffer_list.GetHead() || -len <= total_write_len) {
                break;
            }
            _buffer_write = _buffer_write->GetPrev();
        }
    }
    _can_read_length += total_write_len;
    return total_write_len;
}

uint32_t BufferQueue::ReadUntil(char* res, uint32_t len) {
    if (GetCanReadLength() < len) {
        return 0;

    } else {
        return Read(res, len);
    }
}
    
uint32_t BufferQueue::ReadUntil(char* res, uint32_t len, const char* find, uint32_t find_len, uint32_t& need_len) {
    uint32_t size = FindStr(find, find_len);
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
    
uint32_t BufferQueue::GetCanWriteLength() {
    if (!_buffer_write) {
        return 0;
    }
    
    std::shared_ptr<BufferBlock> temp = _buffer_write;
    uint32_t total_len = 0;
    while (temp) {
        total_len += temp->GetCanWriteLength();
        if (temp == _buffer_list.GetTail()) {
            break;
        }
        temp = temp->GetNext();
    }
    return total_len;
}
    
uint32_t BufferQueue::GetCanReadLength() {
    return _can_read_length;
}

uint32_t BufferQueue::GetFreeMemoryBlock(std::vector<Iovec>& block_vec, uint32_t size) {
    void* mem_1 = nullptr;
    void* mem_2 = nullptr;
    uint32_t mem_len_1 = 0;
    uint32_t mem_len_2 = 0;

    block_vec.reserve(__block_vec_default_size);
    std::shared_ptr<BufferBlock> temp = _buffer_write;
    uint32_t cur_len = 0;
    if (size > 0) {
        while (cur_len < size) {
            if (temp == nullptr) {
                Append();
                temp = _buffer_list.GetTail();
            }
        
            temp->GetFreeMemoryBlock(mem_1, mem_len_1, mem_2, mem_len_2);
            if (mem_len_1 > 0) {
                block_vec.emplace_back(Iovec(mem_1, mem_len_1));
                cur_len += mem_len_1;
            }
            if (mem_len_2 > 0) {
                block_vec.emplace_back(Iovec(mem_2, mem_len_2));
                cur_len += mem_len_2;
            }
            temp = temp->GetNext();
        }

    } else {
        // add one block
        if (!temp) {
            Append();
            temp = _buffer_list.GetTail();
        }
        
        while (temp) {
            temp->GetFreeMemoryBlock(mem_1, mem_len_1, mem_2, mem_len_2);
            if (mem_len_1 > 0) {
                block_vec.emplace_back(Iovec(mem_1, mem_len_1));
                cur_len += mem_len_1;
            }
            if (mem_len_2 > 0) {
                block_vec.emplace_back(Iovec(mem_2, mem_len_2));
                cur_len += mem_len_2;
            }
            if (temp == _buffer_list.GetTail()) {
                break;
            }
            temp = temp->GetNext();
        }
    }
    return cur_len;
}

uint32_t BufferQueue::GetUseMemoryBlock(std::vector<Iovec>& block_vec, uint32_t max_size) {
    void* mem_1 = nullptr;
    void* mem_2 = nullptr;
    uint32_t mem_len_1 = 0;
    uint32_t mem_len_2 = 0;

    block_vec.reserve(__block_vec_default_size);
    std::shared_ptr<BufferBlock> temp = _buffer_list.GetHead();
    uint32_t cur_len = 0;
    while (temp) {
        temp->GetUseMemoryBlock(mem_1, mem_len_1, mem_2, mem_len_2);
        if (mem_len_1 > 0) {
            block_vec.emplace_back(Iovec(mem_1, mem_len_1));
            cur_len += mem_len_1;
        }
        if (mem_len_2 > 0) {
            block_vec.emplace_back(Iovec(mem_2, mem_len_2));
            cur_len += mem_len_2;
        }
        if (temp == _buffer_write) {
            break;
        }
        if (max_size > 0 && cur_len >= max_size) {
            break;
        }
        temp = temp->GetNext();
    }
    return cur_len;
}

uint32_t BufferQueue::FindStr(const char* s, uint32_t s_len) {
    if (_buffer_list.Size() == 0) {
        return 0;
    }

    std::shared_ptr<BufferBlock> temp = _buffer_list.GetHead();
    uint32_t cur_len = 0;
    uint32_t can_read = 0;
    while (temp) {
        can_read = temp->FindStr(s, s_len);
        if (can_read > 0) {
            cur_len += can_read;
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

std::shared_ptr<BlockMemoryPool> BufferQueue::GetBlockMemoryPool() {
    return _block_alloter;
}

void BufferQueue::Reset() {
    _buffer_list.Clear();
    _buffer_write.reset();
}

void BufferQueue::Append() {
    auto temp = _alloter->PoolNewSharePtr<BufferBlock>(_block_alloter);

    if (!_buffer_write) {
        _buffer_write = temp;
    }
    
    _buffer_list.PushBack(temp);
}

}