// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef COMMON_UTIL_BITMAP
#define COMMON_UTIL_BITMAP

#include <vector>
#include <cstdint>

namespace cppnet {

// bitmap base on array.
// find next valid bit in O(1) time.
class Bitmap {
public:
    Bitmap();
    ~Bitmap();

    // init whit array size.
    // one size means bitmap support 64 bit set.
    bool Init(uint32_t size);

    // return false if input param more than bitmap support size.
    bool Insert(uint32_t index);

    // return true even the bit is not in bitmap
    bool Remove(uint32_t index);
    
    // get min index after input param
    // if return -1, means the bitmap has no value
    int32_t GetMinAfter(uint32_t index = 0);

private:
    std::vector<int64_t> _bitmap;
};

}

#endif