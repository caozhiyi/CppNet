// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef COMMON_NETWORK_ADDRESS
#define COMMON_NETWORK_ADDRESS

#include <string>
#include <cstdint>

namespace cppnet {

enum AddressType {
    AT_IPV4  = 0x1,
    AT_IPV6  = 0x2,
};

class Address {
public:
    Address();
    Address(AddressType at);
    Address(AddressType at, const std::string& ip, uint16_t port);
    Address(const Address& addr);
    ~Address();

    void SetType(AddressType at) { _address_type = at; }
    AddressType GetType() { return _address_type; }

    void SetIp(const std::string& ip);
    const std::string& GetIp() { return _ip; }

    void SetPort(uint16_t port) { _port = port; }
    uint16_t GetPort() { return _port; }

    const std::string AsString();

    friend std::ostream& operator<< (std::ostream &out, Address &addr);
    friend bool operator==(const Address &addr1, const Address &addr2);

private:
    bool IsIpv4(const std::string& ip);
    std::string ToIpv6(const std::string& ip);
    std::string ToIpv4(const std::string& ip);

protected:
    AddressType _address_type;
    std::string _ip;
    uint16_t _port;
};

}

#endif