#include "SocketAddress.hpp"

namespace ArsLexis
{

    SocketAddress::SocketAddress()
    {
        MemSet(&address_, sizeof(address_), 0);
    }

    SocketAddress::~SocketAddress()
    {}
    
    INetSocketAddress::INetSocketAddress(const NetIPAddr& ipAddr, UInt16 port, Int16 addressFamily)
    {
        address().family=addressFamily;
        address().port=NetHToNS(port);
        address().addr=ipAddr;
    }
    
}