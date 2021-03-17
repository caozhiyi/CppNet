#include "cppnet_base.h"

namespace cppnet {

CppNetBase::CppNetBase() {

}

CppNetBase::~CppNetBase() {

}

void CppNetBase::Init(uint32_t thread_num) {

}

void CppNetBase::Dealloc() {

}

void CppNetBase::Join() {

}

uint64_t CppNetBase::SetTimer(uint32_t interval, const std::function<void(void*)>& func, void* param = nullptr, bool always = false) {

}

void CppNetBase::RemoveTimer(uint64_t timer_id) {

}

bool CppNetBase::Connection(const std::string& ip, uint16_t port) {

}

void CppNetBase::OnAccept(std::shared_ptr<Socket> sock, uint32_t err) {

}

void CppNetBase::OnRead(std::shared_ptr<Socket> sock, uint32_t err) {

}

void CppNetBase::OnWrite(std::shared_ptr<Socket> sock, uint32_t err) {

}

}