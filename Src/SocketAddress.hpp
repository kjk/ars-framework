#ifndef __ARSLEXIS_SOCKETADDRESS_HPP__
#define __ARSLEXIS_SOCKETADDRESS_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>
#include <NativeSocks.hpp>

namespace ArsLexis
{

    class SocketAddress 
    {
    protected:
        SocketAddr address_;
        
    public:
        
        SocketAddress();

        operator const SocketAddr* () const
        {return &address_;}
        
        uint_t size() const
        {return sizeof(address_);}
        
        virtual ~SocketAddress() {}
    };
    
    class INetSocketAddress: public SocketAddress
    {
        struct INetSocketAddr {
            short family;
            ushort_t port;
            IPAddr ip;
        };
        
        INetSocketAddr& address() 
        {return reinterpret_cast<INetSocketAddr&>(address_);}
        
        const INetSocketAddr& address() const
        {return reinterpret_cast<const INetSocketAddr&>(address_);}
        
    public:
        
        INetSocketAddress();
        
        INetSocketAddress(const IPAddr& addr, ushort_t port, short addressFamily=socketAddrINET);

        void setIpAddress(const IPAddr& addr)
        {address().ip=addr;}      

        void setPort(ushort_t port)
        {address().port=hostToNetS(port);}

        void setAddressFamily(short addressFamily)
        {address().family=addressFamily;}

        const IPAddr& ipAddress() const
        {return address().ip;}

        ushort_t port() const
        {return netToHostS(address().port);}
        
    };
    
}

#endif