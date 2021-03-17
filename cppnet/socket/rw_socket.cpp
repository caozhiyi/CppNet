#include "rw_socket.h"
#include "cppnet/cppnet_base.h"
#include "include/cppnet_type.h"
#include "cppnet/cppnet_config.h"
#include "common/network/io_handle.h"
#include "common/alloter/pool_block.h"
#include "common/buffer/buffer_queue.h"
#include "cppnet/event/event_interface.h"
#include "cppnet/event/action_interface.h"
#include "common/alloter/alloter_interface.h"

namespace cppnet {

RWSocket::RWSocket(std::shared_ptr<AlloterWrap> alloter):
    _alloter(alloter) {
    _block_pool = _alloter->PoolNewSharePtr<BlockMemoryPool>(__mem_block_size, __mem_block_add_step);
}

RWSocket::~RWSocket() {

}

void RWSocket::Read() {
    if (!_event) {
        _event = _alloter->PoolNewSharePtr<Event>();
    }

    _event->AddType(ET_READ);
    _event_actions->AddAcceptEvent(_event);
}

void RWSocket::Write(const char* src, uint32_t len) {
    if (!_event) {
        _event = _alloter->PoolNewSharePtr<Event>();
    }

    _event->AddType(ET_READ);
    _event_actions->AddAcceptEvent(_event);
}

void RWSocket::Connect(const std::string& ip, uint16_t port) {

}

void RWSocket::Disconnect() {

}

void RWSocket::OnRead(uint32_t len) {

}

void RWSocket::OnWrite(uint32_t len) {

}

void RWSocket::OnConnect(uint16_t err) {

}

void RWSocket::Recv() {
    auto base = _cppnet_base.lock();
    if (!base) {
        return;
    }

    uint32_t off_set = 0;
    //read all data.
    uint32_t expand_buff_len = __linux_read_buff_expand_len;
    while (true) {
        uint32_t expand = 0;
        if (_read_buffer->GetCanWriteLength() == 0) {
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
                break;

            } else if (errno == EBADMSG || errno == ECONNRESET) {
                base->OnDisConnect(shared_from_this(), CEC_CONNECT_BREAK);

            } else {
                base->OnDisConnect(shared_from_this(), CEC_CLOSED);
            }

        } else {
            _read_buffer->MoveWritePt(ret._return_value);
            off_set += ret._return_value;
            // read all
            if (ret._return_value < buff_len) {
                break;
            }
        }
    }
    base->OnRead(shared_from_this(), off_set);
}

void RWSocket::Send() {
    auto base = _cppnet_base.lock();
    if (!base) {
        return;
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
                if (ret._return_value < data_len) {
                    _event->AddType(ET_READ);
                    _event_actions->AddAcceptEvent(_event);
                }

            } else if (errno == EBADMSG) {
                base->OnDisConnect(shared_from_this(), CEC_CONNECT_BREAK);
                break;

            } else {
                base->OnDisConnect(shared_from_this(), CEC_CLOSED);
                break;
            }
        }
    }
    base->OnWrite(shared_from_this(), off_set);
}

}