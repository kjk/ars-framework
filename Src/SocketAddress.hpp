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
        
        virtual ~SocketAddress();
    };
    
    class INetSocketAddress: public SocketAddress
    {
        NativeSockAddrIN_t& address() 
        {return reinterpret_cast<NativeSockAddrIN_t&>(address_);}
        
        const NativeSockAddrIN_t& address() const
        {return reinterpret_cast<const NativeSockAddrIN_t&>(address_);}
        
    public:
        
        INetSocketAddress();
        
        INetSocketAddress(const NativeIPAddr_t& ipAddr, ushort_t port, short addressFamily=SocketAddrINET_c);

        void setIpAddress(const NativeIPAddr_t& ipAddr);        

        void setPort(ushort_t port);

        void setAddressFamily(short addressFamily);        

        const NativeIPAddr_t& ipAddress() const;

        ushort_t port() const ;
    };
    
}

#endif