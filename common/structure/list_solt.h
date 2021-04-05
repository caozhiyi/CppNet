#ifndef COMMON_STRUCTURE_LIST_SOLT
#define COMMON_STRUCTURE_LIST_SOLT

#include <memory>

namespace cppnet {

template<typename T>
class ListSolt {
public:
    ListSolt() {}
    virtual ~ListSolt() {}

    void SetNext(std::shared_ptr<T> v) { _next = v; }
    std::shared_ptr<T> GetNext() { return _next; }

    void SetPrev(std::shared_ptr<T> v) { _prev = v; }
    std::shared_ptr<T> GetPrev() { return _prev.lock(); }

protected:
    std::weak_ptr<T>   _prev;
    std::shared_ptr<T> _next;
};

}

#endif