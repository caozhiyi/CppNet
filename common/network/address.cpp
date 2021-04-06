#include <ostream>
#include "address.h"

namespace cppnet {

Address::Address():
    Address(AT_IPV4) {

}

Address::Address(AddressType at):
    _address_type(at),    
    _ip(""),           
    _port(0) {
    
}

Address::Address(AddressType at, const std::string& ip, uint16_t port):
    _address_type(at),
    _ip(ip),
    _port(port) {

}

Address::Address(const Address& addr):
    _address_type(addr._address_type),
    _ip(addr._ip),
    _port(addr._port) {

}

Address::~Address() {

}

void Address::SetIp(const std::string& ip) {
    _ip = ip;
}

std::string Address::GetIp() {
    return _ip;
}

void Address::SetPort(uint16_t port) {
    _port = port;
}

uint16_t Address::GetPort() {
    return _port;
}

const std::string Address::AsString() {
    return std::move(_ip + ":" + std::to_string(_port));
}

std::ostream& operator<< (std::ostream &out, Address &addr) {
    const std::string str = addr.AsString();
    out.write(str.c_str(), str.length());
    return out;
}

bool operator==(const Address &addr1, const Address &addr2) {
    return addr1._ip == addr2._ip && addr1._port == addr2._port && addr1._port != 0;
}

}