// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#include <random>
#include "random.h"

namespace cppnet {

std::random_device RangeRandom::_random;
std::mt19937 RangeRandom::_engine(_random());

RangeRandom::RangeRandom(int32_t min, int32_t max):
    _uniform(min, max) {

}

RangeRandom::~RangeRandom() {

}

int32_t RangeRandom::Random() {
    return _uniform(_engine);
}

}