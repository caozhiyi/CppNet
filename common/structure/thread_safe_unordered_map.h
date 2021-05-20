// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef COMMON_STRUCTURE_THREAD_SAFE_QUEUE
#define COMMON_STRUCTURE_THREAD_SAFE_QUEUE

#include <mutex>
#include <unordered_map>

namespace cppnet {

template<typename K, typename V>
class ThreadSafeUnorderedMap {
public:
    ThreadSafeUnorderedMap() {}
    ~ThreadSafeUnorderedMap() {}

    V& operator[] (const K& key) {
        std::lock_guard<std::mutex> lock(_mutex);
        return _unordered_map[key];
    }

    bool Find(const K& key) {
        std::lock_guard<std::mutex> lock(_mutex);
        return _unordered_map.find(key) != _unordered_map.end();
    }

    void Insert(const std::pair<K, V>& item) {
        std::lock_guard<std::mutex> lock(_mutex);
        _unordered_map.insert(item);
    }

    void Erase(const K& key) {
        std::lock_guard<std::mutex> lock(_mutex);
        _unordered_map.erase(key);
    }

    void Clear() {
        std::lock_guard<std::mutex> lock(_mutex);
        _unordered_map.clear();
    }

    size_t Size() {
        std::lock_guard<std::mutex> lock(_mutex);
        return _unordered_map.size();
    }

    bool Empty() {
        std::lock_guard<std::mutex> lock(_mutex);
        return _unordered_map.empty();
    }

    std::unordered_map<K, V>& GetMap() {
        return _unordered_map;
    }

private:
    std::unordered_map<K, V> _unordered_map;
    std::mutex               _mutex;
};

}

#endif