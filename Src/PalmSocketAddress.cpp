#include <SocketAddress.hpp>

namespace ArsLexis
{

    SocketAddress::SocketAddress()
    {
        MemSet(&address_, sizeof(address_), 0);
    }

    SocketAddress::~SocketAddress()
    {}
    
    INetSocketAddress::INetSocketAddress(const NativeIPAddr_t& ipAddr, ushort_t port, short addressFamily)
    {
        address().family=addressFamily;
        address().port=netToHostS(port);
        address().addr=ipAddr;
    }
    
    INetSocketAddress::INetSocketAddress()
    {
        address().family=SocketAddrINET_c;
        address().port=0;
        address().addr=0;
    
    }   
    
    void INetSocketAddress::setIpAddress(const NativeIPAddr_t& ipAddr)
     {address().addr=ipAddr;}
        
     void INetSocketAddress::setPort(ushort_t port)
     {address().port=hostToNetS(port);}

     void INetSocketAddress::setAddressFamily(short addressFamily)
     {address().family=addressFamily;}
        
     const NativeIPAddr_t& INetSocketAddress::ipAddress() const
     {return address().addr;}
        
     ushort_t INetSocketAddress::port() const 
     {return  netToHostS(address().port);}

}