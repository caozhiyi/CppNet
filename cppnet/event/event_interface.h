#ifndef CPPNET_EVENT_EVENT_INTERFACE
#define CPPNET_EVENT_EVENT_INTERFACE

#include <memory>
#include <atomic>

namespace cppnet {
    enum EventType {
        ET_READ             = 0x001,        // read event
        ET_WRITE            = 0x002,        // write event
        ET_ACCEPT           = 0x004,        // accept event
        ET_TIMER            = 0x008,        // timer event
        ET_USER_TIMER       = 0x010,        // timer event
        ET_CONNECT          = 0x020,        // connect event
        ET_DISCONNECT       = 0x040,        // disconnect event

        ET_INACTIONS        = 0x080,        // setted to actions
    };

    class Socket;
    class Event {
    public:
        Event(): _data(nullptr), _event_type(0) {}
        virtual ~Event() {}

        void SetData(void* data) { _data = data; }
        void* GetData() { return _data; }

        void AddType(EventType type) { _event_type |= type; }
        void RemoveType(EventType type) { _event_type &= ~type; }
        uint16_t GetType() { return _event_type; }
        void ClearType() { _event_type = 0; }

        void SetSocket(std::shared_ptr<Socket> socket) { _socket = socket; }
        std::shared_ptr<Socket> GetSocket() { return _socket.lock(); }

    private:
        void*    _data;
        uint16_t _event_type;
        std::weak_ptr<Socket> _socket;
    };
}

#endif