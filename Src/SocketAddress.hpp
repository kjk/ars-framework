#ifndef __ARSLEXIS_SOCKETADDRESS_HPP__
#define __ARSLEXIS_SOCKETADDRESS_HPP__

#include "Debug.hpp"
#include <utility>

namespace ArsLexis
{

    class SocketAddress 
    {
    public:

        typedef std::pair<NetSocketAddrType*, UInt16> NetSocketAddrProxy;
        typedef std::pair<const NetSocketAddrType*, UInt16> ConstNetSocketAddrProxy;
        
        virtual ConstNetSocketAddrProxy getNetSocketAddrProxy() const=0;
        virtual NetSocketAddrProxy getNetSocketAddrProxy()=0;
        
        virtual ~SocketAddress();
    };
    
    struct INetSocketAddress: public SocketAddress
    {
        NetSocketAddrINType address_;
    public:
        
        INetSocketAddress();
        INetSocketAddress(const NetIPAddr& ipAddr, UInt16 port, Int16 addressFamily=netSocketAddrINET);

        void setIpAddress(const NetIPAddr& ipAddr)
        {address_.addr=ipAddr;}
        
        void setPort(UInt16 port)
        {address_.port=NetHToNS(port);}

        void setAddressFamily(Int16 addressFamily)
        {address_.family=addressFamily;}
        
        const NetIPAddr& ipAddress() const
        {return address_.addr;}
        
        UInt16 port() const 
        {return NetNToHS(address_.port);}
        
        virtual ConstNetSocketAddrProxy getNetSocketAddrProxy() const;
        virtual NetSocketAddrProxy getNetSocketAddrProxy();

    };
    
}


#endif