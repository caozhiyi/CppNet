// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#include <cmath>
#include "bitmap.h"

namespace cppnet {

static const uint32_t __step_size = sizeof(int64_t) * 8;
static const uint64_t __setp_base = 1;

Bitmap::Bitmap():
    _vec_bitmap(0) {

}

Bitmap::~Bitmap() {

}

bool Bitmap::Init(uint32_t size) {
    uint32_t vec_size = size / __step_size;
    // too large size
    if (vec_size > sizeof(_vec_bitmap) * 8) {
        return false;
    }
    if (size % __step_size > 0) {
        vec_size++;
    }
    _bitmap.resize(vec_size);
    for (std::size_t i = 0; i < _bitmap.size(); i++) {
        _bitmap[i] = 0;
    }
    return true;
}

bool Bitmap::Insert(uint32_t index) {
    if (index > _bitmap.size() * __step_size) {
        return false;
    }

    // get index in vector
    uint32_t bitmap_index = index / __step_size;
    // get index in uint64_t
    uint32_t bit_index = index % __step_size;

    _bitmap[bitmap_index] |= __setp_base << bit_index;
    _vec_bitmap |= __setp_base << bitmap_index;

    return true;
}

bool Bitmap::Remove(uint32_t index) {
    if (index > _bitmap.size() * __step_size) {
        return false;
    }

    // get index in vector
    uint32_t bitmap_index = index / __step_size;
    // get index in uint64_t
    uint32_t bit_index = index % __step_size;

    _bitmap[bitmap_index] &= ~(__setp_base << bit_index);
    if (_bitmap[bitmap_index] == 0) {
        _vec_bitmap &= ~(__setp_base << bitmap_index);
    }
    return true;
}

int32_t Bitmap::GetMinAfter(uint32_t index) {
    // get next bit.
    if (index >= _bitmap.size() * __step_size || Empty()) {
        return -1;
    }

    // get index in vector
    uint32_t bitmap_index = index / __step_size;
    // filter smaller bitmap index
    uint32_t ret = bitmap_index * __step_size;

    // find current uint64_t have next 1?
    if (_bitmap[bitmap_index] != 0) {
        int64_t cur_bitmap = _bitmap[bitmap_index];
        int32_t cur_step = index - ret;
        cur_bitmap = cur_bitmap >> cur_step;

        // don't have next 1
        if (cur_bitmap == 0) {
            ret += __step_size;

        // find next 1
        } else {
            ret += cur_step;
            ret += (uint32_t)std::log2f(float(cur_bitmap & (-cur_bitmap)));
            return ret;
        }

    } else {
        ret += __step_size;
    }

    // find next used vector index 
    int32_t temp_vec_bitmap = _vec_bitmap >> bitmap_index;
    if (temp_vec_bitmap == 0) {
        return -1;
    }

    uint32_t next_vec_index = (uint32_t)std::log2f(float(temp_vec_bitmap & (-temp_vec_bitmap) + 1));
    uint32_t target_vec_index = next_vec_index + bitmap_index;
    if (target_vec_index == bitmap_index) {
        return -1;
    }

    int64_t cur_bitmap = _bitmap[target_vec_index];
    ret += (next_vec_index - 1) * __step_size;
    ret += (uint32_t)std::log2f(float(cur_bitmap & (-cur_bitmap) + 1));

    return ret;
}

bool Bitmap::Empty() {
    return _vec_bitmap == 0;
}

void Bitmap::Clear() {
    while (_vec_bitmap != 0) {
        int32_t next_vec_index = (int32_t)std::log2f(float(_vec_bitmap & (-(int32_t)_vec_bitmap) + 1));
        _bitmap[next_vec_index] = 0;
        _vec_bitmap = _vec_bitmap & (_vec_bitmap - 1);
    }
}

}