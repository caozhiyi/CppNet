// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef COMMON_ALLOTER_TREHAD_SAFE_ALLOTER
#define COMMON_ALLOTER_TREHAD_SAFE_ALLOTER

#include <mutex>
#include <memory>
#include <cstdint>

namespace cppnet {

class Alloter;
class TreahdSafeAlloterWrap {
public:
    TreahdSafeAlloterWrap(std::shared_ptr<Alloter> a) : _alloter(a) {}
    ~TreahdSafeAlloterWrap() {}

    //for object. invocation of constructors and destructors
    template<typename T, typename... Args >
    T* PoolNew(Args&&... args);
    template<typename T, typename... Args >
    std::shared_ptr<T> PoolNewSharePtr(Args&&... args);

    template<typename T>
    void PoolDelete(T* &c);

    //for continuous memory
    template<typename T>
    T* PoolMalloc(uint32_t size);
    template<typename T>
    std::shared_ptr<T> PoolMallocSharePtr(uint32_t size);

    template<typename T>
    void PoolFree(T* &m, uint32_t len);

private:
    std::mutex _mutex;
    std::shared_ptr<Alloter> _alloter;
};

template<typename T, typename... Args>
T* TreahdSafeAlloterWrap::PoolNew(Args&&... args) {
    uint32_t sz = sizeof(T);
    
    {
        std::lock_guard<std::mutex> lock(_mutex);
        void* data = _alloter->MallocAlign(sz);
        if (!data) {
            return nullptr;
        }
    }
    
    T* res = new(data) T(std::forward<Args>(args)...);
    return res;
}

template<typename T, typename... Args >
std::shared_ptr<T> TreahdSafeAlloterWrap::PoolNewSharePtr(Args&&... args) {
    {
        std::lock_guard<std::mutex> lock(_mutex);
        T* ret = PoolNew<T>(std::forward<Args>(args)...);
    }
    return std::shared_ptr<T>(ret, [this](T* &c) { PoolDelete(c); });
}

template<typename T>
void TreahdSafeAlloterWrap::PoolDelete(T* &c) {
    if (!c) {
        return;
    }

    c->~T();

    uint32_t len = sizeof(T);
    void* data = (void*)c;

    std::lock_guard<std::mutex> lock(_mutex);
    _alloter->Free(data, len);
    c = nullptr;
}
    
template<typename T>
T* TreahdSafeAlloterWrap::PoolMalloc(uint32_t sz) {
    std::lock_guard<std::mutex> lock(_mutex);
    return (T*)_alloter->MallocAlign(sz);
}

template<typename T>
std::shared_ptr<T> TreahdSafeAlloterWrap::PoolMallocSharePtr(uint32_t size) {
    {
        std::lock_guard<std::mutex> lock(_mutex);
        T* ret = PoolMalloc<T>(size);
    }
    
    return std::shared_ptr<T>(ret, [this, size](T* &c) { PoolFree(c, size); });
}
    
template<typename T>
void TreahdSafeAlloterWrap::PoolFree(T* &m, uint32_t len) {
    if (!m) {
        return;
    }
    void* data = (void*)m;

    std::lock_guard<std::mutex> lock(_mutex);
    _alloter->Free(data, len);
    m = nullptr;
}

}

#endif 