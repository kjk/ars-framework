#include <SocketAddress.hpp>

namespace ArsLexis
{

    SocketAddress::SocketAddress()
    {
        memset(&address_, sizeof(address_), 0);
    }

    SocketAddress::~SocketAddress()
    {}
    
    INetSocketAddress::INetSocketAddress(const NativeIPAddr_t& ipAddr, ushort_t port, short addressFamily)
    {
        address().sin_family=addressFamily;
        address().sin_port=netToHostS(port);
        address().sin_addr=ipAddr;
    }
    
    INetSocketAddress::INetSocketAddress()
    {
        address().sin_family=SocketAddrINET_c;
        address().sin_port=0;
        address().sin_addr.s_addr=0;
    
    }   
    
    void INetSocketAddress::setIpAddress(const NativeIPAddr_t& ipAddr)
     {address().sin_addr=ipAddr;}
        
     void INetSocketAddress::setPort(ushort_t port)
     {address().sin_port=hostToNetS(port);}

     void INetSocketAddress::setAddressFamily(short addressFamily)
     {address().sin_family=addressFamily;}
        
     const NativeIPAddr_t& INetSocketAddress::ipAddress() const
     {return address().sin_addr;}
        
     ushort_t INetSocketAddress::port() const 
     {return  netToHostS(address().sin_port);}
}