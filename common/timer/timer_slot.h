// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef COMMON_TIMER_TIMER_SLOT
#define COMMON_TIMER_TIMER_SLOT

#include <cstdint>
#include "timer_interface.h"

namespace cppnet {

enum TIME_INDEX_TYPE: uint16_t {
    TIT_MILLISECOND = 0x01 << 10,
    TIT_SECOND      = 0x02 << 10,
    TIT_MINUTE      = 0x04 << 10,
    TIT_MUSK        = 0x07 << 10,
};

// Inherit this class to add to timer.
// don't call any function in this class, 
// they internal used by timer.
class TimerSlot {
public:
    TimerSlot();
    ~TimerSlot() {}

//private:
public:
    enum TIMER_SOLT_FLAG: uint32_t {
        TSF_IN_TIMER = (uint32_t)1 << 30,
        TSF_ALWAYS   = (uint32_t)1 << 31,
    };

    // timer out call back
    virtual void OnTimer() = 0;

    void SetInterval(uint32_t interval);
    uint32_t GetTotalInterval();
    uint32_t GetLeftInterval();

    void ResetTime();
    uint32_t TimePass(uint32_t time);

    void SetInTimer();
    bool IsInTimer();
    void RmInTimer();

    void SetAlways();
    bool IsAlways();
    void RmAlways();

    void SetCurIndex(uint16_t index, uint16_t type);
    void GetCurIndex(uint16_t& index, uint16_t& type);

private:
    friend class TimerContainer;

    uint32_t _total_interval;
    uint32_t _left_interval;

    uint16_t _cur_index;
};

}

#endif