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
    Address(AddressType at);
    Address(AddressType at, const std::string& ip, uint16_t port);
    ~Address();

    virtual void SetIp(const std::string& ip);
    virtual std::string GetIp();

    virtual void SetPort(uint16_t port);
    virtual uint16_t GetPort();

    virtual const std::string AsString();

    friend std::ostream& operator<< (std::ostream &out, Address &addr);
    friend bool operator==(const Address &addr1, const Address &addr2);

protected:
    AddressType _address_type;
    std::string _ip;
    uint16_t _port;
};

}

#endif