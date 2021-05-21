// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef CPPNET_EVENT_WIN_RW_EVENT
#define CPPNET_EVENT_WIN_RW_EVENT

#include "cppnet/event/event_interface.h"

namespace cppnet {

class BufferQueue;
class WinRWEvent:
    public Event{
public:
    WinRWEvent(): 
       _ex_data(nullptr) {}
    virtual ~WinRWEvent() {}

    void SetExData(void* data) { _ex_data = data; }
    void* GetExData() { return _ex_data; }

    void SetBuffer(std::shared_ptr<BufferQueue>& buffer) { _buffer = buffer; }
    std::shared_ptr<BufferQueue> GetBuffer() { return _buffer; }

private:
    void* _ex_data;

private:
    std::shared_ptr<BufferQueue> _buffer;
};

}

#endif