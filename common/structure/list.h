#ifndef COMMON_STRUCTURE_LIST
#define COMMON_STRUCTURE_LIST

#include <memory>
#include <cstdint>
#include "list_solt.h"

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