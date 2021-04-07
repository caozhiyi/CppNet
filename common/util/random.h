// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef COMMON_UTIL_RANDOM
#define COMMON_UTIL_RANDOM

#include <random>
#include <cstdint>

namespace cppnet {

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

}

#endif