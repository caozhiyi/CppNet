#include <cmath>
#include "bitmap.h"

namespace cppnet {

static const uint32_t __step_size = sizeof(int64_t) * 8;
static const uint32_t __max_size = __step_size * 5;
static const uint64_t __bit_base = 1;

Bitmap::Bitmap() {
    
}

Bitmap::~Bitmap() {

}

bool Bitmap::Init(uint32_t size) {
    if (size > __max_size) {
        return false;
    }

    uint32_t vec_size = size / __step_size;
    if (size % __step_size > 0) {
        vec_size++;
    }
    _bitmap.resize(vec_size);
    return true;
}

bool Bitmap::Insert(uint32_t index) {
    if (index > _bitmap.size() * __step_size) {
        return false;
    }

    uint32_t step = index;
    for (std::size_t i = 0; i < _bitmap.size(); i++) {
        if (index < (i + 1) * __step_size) {
            _bitmap[i] = _bitmap[i] | (__bit_base << step);
            return true;
        }
        step -= __step_size;
    }
    return false;
}

bool Bitmap::Remove(uint32_t index) {
    if (index > _bitmap.size() * __step_size) {
        return false;
    }

    uint32_t step = index;
    for (std::size_t i = 0; i < _bitmap.size(); i++) {
        if (index < (i + 1) * __step_size) {
            _bitmap[i] = _bitmap[i] & (~(__bit_base << step));
            return true;
        }
        step -= __step_size;
    }
    return false;
}

int32_t Bitmap::GetMinAfter(uint32_t index) {
    // get next bit.
    if (index != 0) {
        index++;
    }
    if (index > _bitmap.size() * __step_size) {
        return -1;
    }

    uint32_t ret = 0;
    for (std::size_t i = 0; i < _bitmap.size(); i++) {
        if (index > (i + 1) * __step_size) {
            ret += __step_size;

        } else {
            if (_bitmap[i] == 0) {
                ret += __step_size;
                continue;
            }
            
            if (index > ret) {
                int64_t cur_bitmap = _bitmap[i];
                int32_t cur_step = index - ret;
                cur_bitmap = cur_bitmap >> cur_step;
                if (cur_bitmap == 0) {
                    ret += __step_size;
                    continue;
                }
                ret += cur_step;
                ret += (uint32_t)std::log2f(float(cur_bitmap & (-cur_bitmap)));
                return ret;

            } else {
                int64_t cur_bitmap = _bitmap[i];
                if (cur_bitmap == 0) {
                    ret += __step_size;
                    continue;
                }
                ret += (uint32_t)std::log2f(float(cur_bitmap & (-cur_bitmap)));
                return ret;
            }
           
        }
    }
    return -1;
}

}