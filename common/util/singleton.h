// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef COMMON_UTIL_SINGLETON
#define COMMON_UTIL_SINGLETON

// VENDORED-FROM: quicX/src/common/util/singleton.h (CppNet adapted)
// Stage 3.1 of upgrade: relocated to cppnet::common namespace.
// The old cppnet::Singleton symbol is preserved via a using-declaration
// at the end of this header so existing CppNet business code keeps working
// without modification (self-export pattern, see common/log).

namespace cppnet {
namespace common {

template<typename T>
class Singleton {
public:
    static T& Instance() {
        static T instance;
        return instance;
    }

protected:
    Singleton(const Singleton&) {}
    Singleton& operator = (const Singleton&) {}
    Singleton() {}
    virtual ~Singleton() {}
};

}  // namespace common

// Self-export: keep cppnet::Singleton<T> working for existing call sites.
using common::Singleton;

}  // namespace cppnet
#endif