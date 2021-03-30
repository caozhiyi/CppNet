#include <errno.h>
#include "posix_rw_socket.h"

#include "cppnet/cppnet_base.h"
#include "cppnet/cppnet_config.h"
#include "cppnet/event/event_interface.h"
#include "cppnet/event/action_interface.h"

#include "common/buffer/buffer_queue.h"

namespace cppnet {

std::shared_ptr<RWSocket> MakeRWSocket(std::shared_ptr<AlloterWrap> alloter) {
    return std::make_shared<PosixRWSocket>(alloter);
}

std::shared_ptr<RWSocket> MakeRWSocket(uint64_t sock, std::shared_ptr<AlloterWrap> alloter) {
    return std::make_shared<PosixRWSocket>(sock, alloter);
}

PosixRWSocket::PosixRWSocket(std::shared_ptr<AlloterWrap> alloter): 
    RWSocket(alloter) {

}

PosixRWSocket::PosixRWSocket(uint64_t sock, std::shared_ptr<AlloterWrap> alloter):
    RWSocket(sock, alloter) {

}

PosixRWSocket::~PosixRWSocket() {

}

bool PosixRWSocket::Write(const char* src, uint32_t len) {
    if (!_event) {
        _event = _alloter->PoolNewSharePtr<Event>();
        _event->SetSocket(shared_from_this());
    }

    //can't send now
    if (_write_buffer->GetCanReadLength() > 0) {
        if (_write_buffer->GetCanReadLength() > __max_write_cache) {
            return false;
        }
        
        _write_buffer->Write(src, len);
        auto actions = GetEventActions();
        if (actions) {
            return actions->AddSendEvent(_event);
        }
        return false;

    } else {
        _write_buffer->Write(src, len);
        return Send();
    }
}

void PosixRWSocket::OnRead(uint32_t len) {
    Recv(len);
}

void PosixRWSocket::OnWrite(uint32_t len) {
    Send();
}

bool PosixRWSocket::Recv(uint32_t len) {
    auto cppnet_base = _cppnet_base.lock();
    if (!cppnet_base) {
        return false;
    }
    if (len == 0) {
        len = __linux_read_buff_expand_len;
    }

    uint32_t off_set = 0;
    //read all data.
    uint32_t expand_buff_len = len;
    bool need_expend = false;
    while (true) {
        uint32_t expand = 0;
        if (need_expend) {
            expand = expand_buff_len;
            if (expand_buff_len < __linux_read_buff_expand_max) {
                expand_buff_len *= 2;
            }
        }

        std::vector<Iovec> io_vec;
        uint32_t buff_len = _read_buffer->GetFreeMemoryBlock(io_vec, expand);
        auto ret = OsHandle::Readv(_sock, &*io_vec.begin(), io_vec.size());
        if (ret._return_value < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR) {
                _read_buffer->MoveWritePt(ret._return_value);
                off_set += ret._return_value;
                break;

            } else {
                OnDisConnect(CEC_CONNECT_BREAK);
                return false;
            }

        } else if (ret._return_value == 0) {
            OnDisConnect(CEC_CLOSED);
            return false;

        } else {
            _read_buffer->MoveWritePt(ret._return_value);
            off_set += ret._return_value;
            // read all
            if (ret._return_value < buff_len) {
                break;
            }
            need_expend = true;
        }
    }
    cppnet_base->OnRead(shared_from_this(), off_set);
    return true;
}

bool PosixRWSocket::Send() {
    auto cppnet_base = _cppnet_base.lock();
    if (!cppnet_base) {
        return false;
    }

    uint32_t off_set = 0;
    while(_write_buffer && _write_buffer->GetCanReadLength() > 0) {
        std::vector<Iovec> io_vec;
        uint32_t data_len = _write_buffer->GetUseMemoryBlock(io_vec, __linux_write_buff_get);
        auto ret = OsHandle::Writev(_sock, &*io_vec.begin(), io_vec.size());
        if (ret._return_value >= 0) {
            _write_buffer->MoveReadPt(ret._return_value);
            off_set += ret._return_value;

        } else {
            if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR) {
                //can't send complete
                auto actions = GetEventActions();
                if (actions) {
                    return actions->AddSendEvent(_event);
                }
                return false;

            } else if (errno == EBADMSG) {
                OnDisConnect(CEC_CONNECT_BREAK);
                return false;

            } else {
                OnDisConnect(CEC_CLOSED);
                return false;
            }
        }
    }
    cppnet_base->OnWrite(shared_from_this(), off_set);
    return true;
}

}