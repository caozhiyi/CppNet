#include "include/cppnet_socket.h"

namespace cppnet {

CNSocket::CNSocket() {

}

CNSocket::~CNSocket() {

}

bool CNSocket::GetAddress(std::string& ip, uint16_t& port) { 
    return true; 
}

bool CNSocket::Write(const char* src, uint32_t len) { 
    return true; 
}

bool CNSocket::Close() { 
    return true; 
}

uint64_t CNSocket::AddTimer(uint32_t interval, bool always) { 
    return 0; 
}

void CNSocket::StopTimer(uint64_t timer_id) {

}

}
