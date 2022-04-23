// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef COMMON_STRUCTURE_LIST
#define COMMON_STRUCTURE_LIST

#include <memory>
#include <cstdint>
#include "list_slot.h"

namespace cppnet {

template<typename T>
class List {
public:
    List(): _size(0) {}
    ~List() {}

    uint32_t Size() { return _size; }

    std::shared_ptr<T> GetHead() { return _head; }
    std::shared_ptr<T> GetTail() { return _tail; }

    void Clear() {
        _size = 0;

        _head.reset();
        _tail.reset();
    }

    void PushBack(std::shared_ptr<T> v) {
        if (!v) {
            return;
        }
    
        if (!_tail) {
            _tail = v;
            _head = v;

        } else {
            _tail->SetNext(v);
            v->SetPrev(_tail);
            _tail = v;
        }
        _size++;
    }

    std::shared_ptr<T> PopBack() {
        if (!_tail) {
            return nullptr;
        }

        auto ret = _tail;
        _tail = _tail->GetPrev();
        if (!_tail) {
            _head.reset();

        } else {
            _tail->SetNext(nullptr);
        }
        _size--;

        return ret;
    }

    void PushFront(std::shared_ptr<T> v) {
        if (!v) {
            return;
        }
    
        if (!_head) {
            _tail = v;
            _head = v;

        } else {
            _head->SetPrev(v);
            v->SetNext(_head);
            _head = v;
        }
        _size++;
    }

    std::shared_ptr<T> PopFront() {
        if (!_head) {
            return nullptr;
        }

        auto ret = _head;
        _head = _head->GetNext();
        if (!_head) {
            _tail.reset();
        }
        
        _size--;

        return ret;
    }

private:
    uint32_t _size;
    std::shared_ptr<T> _head;
    std::shared_ptr<T> _tail;
};

}

#endif