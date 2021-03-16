#include "timer_solt.h"

namespace cppnet {

std::unordered_map<uint32_t, uint8_t> TimerSolt::_index_map;

TimerSolt::TimerSolt() {  
    _index._index = 0;
    _index_map[TC_1MS]  =  0;
    _index_map[TC_50MS] =  1;
    _index_map[TC_1SEC] =  2;
    _index_map[TC_1MIN] =  3;
}

TimerSolt::~TimerSolt() {

}

uint8_t TimerSolt::GetIndex(TIMER_CAPACITY tc) {
    auto index = _index_map[tc];
    return _index._index_arr[index] & ~TSF_INDEX_MASK;
}

uint8_t TimerSolt::SetIndex(uint32_t index) {
    if (index > TC_1HOUR) {
        return 0;
    }

    uint8_t ret = 0;
    if (index >= TC_1MIN) {
        uint8_t sub_index = index / TC_1MIN;
        index = index % TC_1MIN;
        SetIndex(3, sub_index);
        ret = sub_index;
    } 
    
    if (index >= TC_1SEC) {
        uint8_t sub_index = index / TC_1SEC;
        index = index % TC_1SEC;
        SetIndex(2, sub_index);
        if (ret == 0) {
            ret = sub_index;
        }
    }

    if (index >= TC_50MS) {
        uint8_t sub_index = index / TC_50MS;
        index = index % TC_50MS;
        SetIndex(1, sub_index);
        if (ret == 0) {
            ret = sub_index;
        }
    }
    
    if (index > 0) {
        uint8_t sub_index = index;
        SetIndex(0, sub_index);
        if (ret == 0) {
            ret = sub_index;
        }
    }
    return ret;
}

void TimerSolt::SetIndex(uint8_t index, TIMER_CAPACITY tc) {
    switch (tc)
    {
    case TC_1MS:
        SetIndex(0, index);
        break;
    case TC_50MS:
        SetIndex(1, index);
        break;
    case TC_1SEC:
        SetIndex(2, index);
        break;
    case TC_1MIN:
        SetIndex(3, index);
        break;
    default:
        // shouldn't here
        break;
    }
}

void TimerSolt::SetAlways(TIMER_CAPACITY tc) {
    auto index = _index_map[tc];
    _index._index_arr[index] |= TSF_ALWAYS;
}

void TimerSolt::CancelAlways(TIMER_CAPACITY tc) {
    auto index = _index_map[tc];
    _index._index_arr[index] &= ~TSF_ALWAYS;
}

bool TimerSolt::IsAlways(TIMER_CAPACITY tc) {
    auto index = _index_map[tc];
    return _index._index_arr[index] & TSF_ALWAYS;
}

void TimerSolt::Clear() {
    _interval = 0; 
    _index._index = 0; 
}

void TimerSolt::SetIndex(uint32_t pos, uint8_t index) {
    // clear current index
    _index._index_arr[pos] &= TSF_INDEX_MASK;
    _index._index_arr[pos] |= index; 
}

void TimerSolt::SetTimer() {
    _index._index_arr[3] |= TSF_INTIMER;
}

void TimerSolt::RmTimer() {
    Clear();
}

bool TimerSolt::IsInTimer() {
    return _index._index_arr[3] & TSF_INTIMER;
}

uint32_t TimerSolt::GetInterval() {
    return _interval;
}

void TimerSolt::SetInterval(uint32_t time) {
    _interval = time;
}

}