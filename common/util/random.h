// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef COMMON_UTIL_RANDOM
#define COMMON_UTIL_RANDOM

// VENDORED-FROM: quicX/src/common/util/random.h (CppNet adapted)
// Stage 3.1 of upgrade: relocated to cppnet::common namespace.
// The old cppnet::RangeRandom symbol is preserved via a using-declaration
// at the end of this header (self-export pattern).

#include <cstdint>
#include <random>

namespace cppnet {
namespace common {

class RangeRandom {
public:
    RangeRandom(int32_t min, int32_t max);
    ~RangeRandom();

    int32_t Random();

private:
    static std::random_device _random;
    static std::mt19937       _engine;
    std::uniform_int_distribution<int32_t> _uniform;
};

}  // namespace common

// Self-export: keep cppnet::RangeRandom working for existing call sites.
using common::RangeRandom;

}  // namespace cppnet

#endif