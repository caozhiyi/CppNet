#ifndef COMMON_TIMER_TIMER_SOLT
#define COMMON_TIMER_TIMER_SOLT

#include <cstdint>
#include <unordered_map>
#include "timer_interface.h"

namespace cppnet {

// Inherit this class to add to timer.
// don't call any function in this class, 
// they internal used by timer.
class TimerSolt {
public:

    enum TIMER_SOLT_FLAG {
        TSF_INDEX_MASK = 3 << 6,
        TSF_ALWAYS     = 1 << 7,
        TSF_INTIMER    = 1 << 6,
    };

    TimerSolt();
    ~TimerSolt();
    // timer out call back
    virtual void OnTimer() = 0;

    uint8_t GetIndex(TIMER_CAPACITY tc);
    uint8_t SetIndex(uint32_t index);
    void SetIndex(uint8_t index, TIMER_CAPACITY tc);

    void SetAlways(TIMER_CAPACITY tc);
    void CancelAlways(TIMER_CAPACITY tc);
    bool IsAlways(TIMER_CAPACITY tc);

    void Clear();

    void SetTimer();
    void RmTimer();
    bool IsInTimer();

    uint32_t GetInterval();
    void SetInterval(uint32_t time);

private:
    void SetIndex(uint32_t pos, uint8_t index);

private:
    static std::unordered_map<uint32_t, uint8_t> _index_map;
    union {
        uint8_t  _index_arr[4];
        uint32_t _index;
    } _index;
    uint32_t _interval;
};

}

#endif