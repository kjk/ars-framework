#include <SocketAddress.hpp>
#if !(defined(_WIN32_WCE) || defined(_WIN32))
#include <cstring>
#endif

SocketAddress::SocketAddress()
{
    memzero(&address_, sizeof(address_));
}

SocketAddress::~SocketAddress()
{}

INetSocketAddress::INetSocketAddress(const IPAddr& addr, ushort_t port, short addressFamily)
{
    address().family=addressFamily;
    address().port=netToHostS(port);
    address().ip=addr;
}

INetSocketAddress::INetSocketAddress()
{
    address().family=socketAddrINET;
    address().port=0;
    address().ip.ip=0;
}


INetSocketAddress::~INetSocketAddress()
{}
