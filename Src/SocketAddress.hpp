#ifndef __ARSLEXIS_SOCKETADDRESS_HPP__
#define __ARSLEXIS_SOCKETADDRESS_HPP__

#include "Debug.hpp"

namespace ArsLexis
{

    class SocketAddress 
    {
    protected:
        NetSocketAddrType address_;        
        
    public:
        
        SocketAddress();

        operator const NetSocketAddrType* () const
        {return &address_;}
        
        UInt16 size() const
        {return sizeof(address_);}
        
        virtual ~SocketAddress();
    };
    
    class INetSocketAddress: public SocketAddress
    {
        NetSocketAddrINType& address() 
        {return reinterpret_cast<NetSocketAddrINType&>(address_);}
        
        const NetSocketAddrINType& address() const
        {return reinterpret_cast<const NetSocketAddrINType&>(address_);}
        
    public:
        
        INetSocketAddress()
        {}
        
        INetSocketAddress(const NetIPAddr& ipAddr, UInt16 port, Int16 addressFamily=netSocketAddrINET);

        void setIpAddress(const NetIPAddr& ipAddr)
        {address().addr=ipAddr;}
        
        void setPort(UInt16 port)
        {address().port=NetHToNS(port);}

        void setAddressFamily(Int16 addressFamily)
        {address().family=addressFamily;}
        
        const NetIPAddr& ipAddress() const
        {return address().addr;}
        
        UInt16 port() const 
        {return NetNToHS(address().port);}
        
    };
    
}


#endif