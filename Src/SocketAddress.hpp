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
        NativeSocketAddr_t address_;
        
    public:
        
        SocketAddress();

        operator const NativeSocketAddr_t* () const
        {return &address_;}
        
        uint_t size() const
        {return sizeof(address_);}
        
        virtual ~SocketAddress() {}
    };
    
    class INetSocketAddress: public SocketAddress
    {
        struct INetSocketAddr {
            short family;
            uint_t port;
            NativeIPAddr_t ip;
        };
        
        INetSocketAddr& address() 
        {return reinterpret_cast<INetSocketAddr&>(address_);}
        
        const INetSocketAddr& address() const
        {return reinterpret_cast<const INetSocketAddr&>(address_);}
        
    public:
        
        INetSocketAddress();
        
        INetSocketAddress(const NativeIPAddr_t& ipAddr, ushort_t port, short addressFamily=socketAddrINET);

        void setIpAddress(const NativeIPAddr_t& ipAddr)
        {address().ip=ipAddr;}      

        void setPort(ushort_t port)
        {address().port=hostToNetS(port);}

        void setAddressFamily(short addressFamily)
        {address().family=addressFamily;}

        const NativeIPAddr_t& ipAddress() const
        {return address().ip;}

        ushort_t port() const
        {return netToHostS(address().port);}
        
    };
    
}

#endif