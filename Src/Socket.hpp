#ifndef __ARSLEXIS_SOCKET_HPP__
#define __ARSLEXIS_SOCKET_HPP__

#include "Debug.hpp"

namespace ArsLexis
{

    class NetLibrary;
    
    class SocketAddress;
    
    class SocketBase
    {
        SocketBase(const SocketBase&);
        SocketBase& operator=(const SocketBase&);
    protected:
    
        NetLibrary& netLib_;

        NetSocketRef socket_;

        SocketBase(NetLibrary& netLib);
        
        ~SocketBase();
                
    public:
        
        Err open(NetSocketAddrEnum domain=netSocketAddrINET,
            NetSocketTypeEnum type=netSocketTypeStream,
            Int16 protocol=0, Int32 timeout=evtWaitForever);

        Err shutdown(Int16 direction, Int32 timeout=evtWaitForever);
        
        Err send(UInt16& sent, const void* buffer, UInt16 bufferLength, Int32 timeout=evtWaitForever, UInt16 flags=0, const SocketAddress* address=0);
        
        Err receive(UInt16& received, void* buffer, UInt16 bufferLength, Int32 timeout=evtWaitForever, UInt16 flags=0);
        
            
    };

    class Socket: public SocketBase
    {
    public:
    
        Socket(NetLibrary& netLib):
            SocketBase(netLib)
        {}
        
        ~Socket()
        {}
        
        Err connect(const SocketAddress& address, Int32 timeout=evtWaitForever);
        
    };
    
}

#endif