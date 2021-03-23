#include <errno.h>

#include "rw_socket.h"
#include "include/cppnet_type.h"

#include "cppnet/dispatcher.h"
#include "cppnet/cppnet_base.h"
#include "cppnet/cppnet_config.h"
#include "cppnet/event/event_interface.h"
#include "cppnet/event/action_interface.h"

#include "common/log/log.h"
#include "common/network/address.h"
#include "common/network/io_handle.h"
#include "common/alloter/pool_block.h"
#include "common/buffer/buffer_queue.h"
#include "common/alloter/alloter_interface.h"

namespace cppnet {
RWSocket::RWSocket(std::shared_ptr<AlloterWrap> alloter): 
    Socket(alloter) {

    _block_pool = _alloter->PoolNewSharePtr<BlockMemoryPool>(__mem_block_size, __mem_block_add_step);
    _write_buffer = _alloter->PoolNewSharePtr<BufferQueue>(_block_pool, _alloter);
    _read_buffer = _alloter->PoolNewSharePtr<BufferQueue>(_block_pool, _alloter);
}

RWSocket::RWSocket(uint64_t sock, std::shared_ptr<AlloterWrap> alloter):
    Socket(sock, alloter) {

    _block_pool = _alloter->PoolNewSharePtr<BlockMemoryPool>(__mem_block_size, __mem_block_add_step);
    _write_buffer = _alloter->PoolNewSharePtr<BufferQueue>(_block_pool, _alloter);
    _read_buffer = _alloter->PoolNewSharePtr<BufferQueue>(_block_pool, _alloter);
}

RWSocket::~RWSocket() {

}

bool RWSocket::GetAddress(std::string& ip, uint16_t& port) {
    if (!_addr) {
        return false;
    }
    
    ip = _addr->GetIp();
    port = _addr->GetPort();

    return true;
}

bool RWSocket::Close() {
    Disconnect();
    return true;
}

void RWSocket::Read() {
    if (!_event) {
        _event = _alloter->PoolNewSharePtr<Event>();
        _event->SetSocket(shared_from_this());
    }

    auto actions = GetEventActions();
    if (actions) {
        actions->AddRecvEvent(_event);
    }
}

bool RWSocket::Write(const char* src, uint32_t len) {
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

void RWSocket::Connect(const std::string& ip, uint16_t port) {
    if (!_event) {
        _event = _alloter->PoolNewSharePtr<Event>();
        _event->SetSocket(shared_from_this());
    }

    if (_sock == 0) {
        auto ret = OsHandle::TcpSocket();
        if (ret._return_value < 0) {
            LOG_ERROR("create socket failed. error:%d", ret._errno);
            return;
        }
        _sock = ret._return_value;
    }

    if (!_addr) {
        _addr = _alloter->PoolNewSharePtr<Address>(AT_IPV4, ip, port);

    } else {
        _addr->SetIp(ip);
        _addr->SetPort(port);
    }

    auto actions = GetEventActions();
    if (actions) {
        actions->AddConnection(_event, *_addr);
    }
}

void RWSocket::Disconnect() {
    if (!_event) {
        _event = _alloter->PoolNewSharePtr<Event>();
        _event->SetSocket(shared_from_this());
    }

    auto actions = GetEventActions();
    if (actions) {
        actions->AddDisconnection(_event);
    }
}

void RWSocket::OnTimer() {
    auto cppnet_base = _cppnet_base.lock();
    if (!cppnet_base) {
        return;
    }
    cppnet_base->OnTimer(shared_from_this());
}

uint64_t RWSocket::AddTimer(uint32_t interval, bool always) {
    auto dispatcher = GetDispatcher();
    if (dispatcher) {
        return dispatcher->AddTimer(shared_from_this(), interval, always);
    }
    return 0;
}

void RWSocket::StopTimer(uint64_t timer_id) {
    auto dispatcher = GetDispatcher();
    if (dispatcher) {
        dispatcher->StopTimer(timer_id);
    }
}

void RWSocket::OnRead(uint32_t len) {
    Recv();
}

void RWSocket::OnWrite(uint32_t len) {
    Send();
}

void RWSocket::OnConnect(uint16_t err) {
    auto sock = shared_from_this();
    if (err == CEC_SUCCESS) {
        __all_socket_map[_sock] = sock;
        Read();
    }
    
    auto cppnet_base = _cppnet_base.lock();
    if (cppnet_base) {
        cppnet_base->OnConnect(sock, err);
    }
}

void RWSocket::OnDisConnect(uint16_t err) {
    auto sock = shared_from_this();
    __all_socket_map.erase(_sock);

    auto cppnet_base = _cppnet_base.lock();
    if (cppnet_base) {
        cppnet_base->OnDisConnect(sock, err);
    }

    // not active disconnection
    if (_event && !(_event->GetType() & ET_DISCONNECT)) {
        OsHandle::Close(_sock);
    }
}

bool RWSocket::Recv() {
    auto cppnet_base = _cppnet_base.lock();
    if (!cppnet_base) {
        return false;
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
                _read_buffer->MoveWritePt(ret._return_value);
                off_set += ret._return_value;
                break;

            } else if (errno == EBADMSG) {
                OnDisConnect(CEC_CONNECT_BREAK);
                return false;

            } else {
                
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
        }
    }
    cppnet_base->OnRead(shared_from_this(), off_set);
    return true;
}

bool RWSocket::Send() {
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