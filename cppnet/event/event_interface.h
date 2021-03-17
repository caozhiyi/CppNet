#ifndef NET_EVENT_EVENT_INTERFACE
#define NET_EVENT_EVENT_INTERFACE

#include <memory>
#include <atomic>

namespace cppnet {
    enum EventType {
        ET_READ             = 0x001,        // read event
        ET_WRITE            = 0x002,        // write event
        ET_ACCEPT           = 0x004,        // accept event
        ET_TIMER            = 0x008,        // timer event
        ET_CONNECT          = 0x010,        // connect event
        ET_DISCONNECT       = 0x020,        // disconnect event

        ET_INACTIONS        = 0x040,        // setted to actions
    };

    enum EventSetFlag {
        ESF_READ             = 0x01,        // setted read event
        ESF_WRITE            = 0x02,        // setted write event
        ESF_CONNECT          = 0x04,        // setted connect event
        ESF_DISCONNECT       = 0x08,        // setted disconnect event
    };

    class Socket;
    class Event {
    public:
        Event(): _data(nullptr), _event_type(0), _event_setted(0) {}
        Event(EventType type): _data(nullptr), _event_type(type), _event_setted(0) {}
        virtual ~Event() {}

        void SetData(void* data) { _data = data; }
        void* GetData() { return _data; }

        void AddType(EventType type) { _event_type |= type; }
        void RemoveType(EventType type) { _event_type &= ~type; }
        uint16_t GetType() { return _event_type; }
        void ClearType() { _event_type = 0; }

        void AddSettedFlag(EventSetFlag flag) { _event_setted |= flag; }
        void RemoveSettedFlag(EventSetFlag flag) { _event_setted &= ~flag; }
        uint16_t GetSettedFlag() { return _event_setted; }
        void ClearSettedFlag() { _event_setted = 0; }

        void SetSocket(std::shared_ptr<Socket> socket) { _socket = socket; }
        std::shared_ptr<Socket> GetSocket() { return _socket.lock(); }

    private:
        void*    _data;
        uint16_t _event_type;
        uint16_t _event_setted;
        std::weak_ptr<Socket> _socket;
    };
}

#endif