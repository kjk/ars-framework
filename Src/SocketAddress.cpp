#include "SocketAddress.hpp"

namespace ArsLexis
{

    SocketAddress::~SocketAddress()
    {}
    
    INetSocketAddress::INetSocketAddress(const NetIPAddr& ipAddr, UInt16 port, Int16 addressFamily)
    {
        MemSet(&address_, sizeof(address_), 0);
        address_.family=addressFamily;
        address_.port=NetHToNS(port);
        address_.addr=ipAddr;
    }
    
    SocketAddress::ConstNetSocketAddrProxy INetSocketAddress::getNetSocketAddrProxy() const
    {
        return ConstNetSocketAddrProxy(reinterpret_cast<const NetSocketAddrType*>(&address_), sizeof(address_));
    }
        
    SocketAddress::NetSocketAddrProxy INetSocketAddress::getNetSocketAddrProxy()
    {
        return NetSocketAddrProxy(reinterpret_cast<NetSocketAddrType*>(&address_), sizeof(address_));
    }

}