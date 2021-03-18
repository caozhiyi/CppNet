#include <thread>
#include "os_info.h"

uint32_t cppnet::GetCpuNum() {
    return std::thread::hardware_concurrency();
}