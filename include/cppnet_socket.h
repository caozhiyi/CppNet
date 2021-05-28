// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef INCLUDE_CPPNET_SOCKET
#define INCLUDE_CPPNET_SOCKET

#include <memory>
#include <string>
#include <cstdint>

namespace cppnet {

// cpp net socket interface
class CNSocket {
public:
    CNSocket();
    virtual ~CNSocket();
    // get socket ip and adress
    virtual bool GetAddress(std::string& ip, uint16_t& port);
    // post sync write event.
    virtual bool Write(const char* src, uint32_t len);
    // close the connect
    virtual bool Close();
    // add a timer. must set timer call back
    // interval support max 1 minute
    // return a timer id
    virtual uint64_t AddTimer(uint32_t interval, bool always = false);
    virtual void StopTimer(uint64_t timer_id);
};

}

#endif