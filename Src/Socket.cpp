#include "Socket.hpp"
#include "SocketAddress.hpp"
#include "NetLibrary.hpp"

namespace ArsLexis
{

    SocketBase::SocketBase(NetLibrary& netLib):
        netLib_(netLib),
        socket_(0)
    {
        assert(netLib_.refNum());
    }
    
    SocketBase::~SocketBase()
    {
        if (socket_!=0)
        {
            Err error=errNone;
            NetLibSocketClose(netLib_, socket_, evtWaitForever, &error);
        }
    }
    
    Err SocketBase::open(NetSocketAddrEnum domain, NetSocketTypeEnum type, Int16 protocol, Int32 timeout)
    {
        assert(!socket_);
        Err error=errNone;
        socket_=NetLibSocketOpen(netLib_, domain, type, protocol, timeout, &error);
        if (!error)
            assert(socket_!=0);
        return error;
    }
    
    Err SocketBase::shutdown(Int16 direction, Int32 timeout)
    {
        assert(socket_!=0);
        Err error=errNone;
        Int16 result=NetLibSocketShutdown(netLib_, socket_, direction, timeout, &error);
        if (result<0)
            assert(error);
        return error;
    }
    
    Err SocketBase::send(UInt16& sent, const void* buffer, UInt16 bufferLength, Int32 timeout, UInt16 flags, const SocketAddress* address)
    {
        assert(socket_!=0);
        Err error=errNone;
        const NetSocketAddrType* addr=0;
        UInt16 addrLen=0;
        if (address)
        {
            SocketAddress::ConstNetSocketAddrProxy proxy=address->getNetSocketAddrProxy();
            addr=proxy.first;
            addrLen=proxy.second;
        }
        Int16 result=NetLibSend(netLib_, socket_, const_cast<void*>(buffer), bufferLength, flags, 
            const_cast<NetSocketAddrType*>(addr), addrLen, timeout, &error);
        if (result>0)
        {
            assert(!error);
            sent+=result;
        }
        else if (0==result)
            assert(netErrSocketClosedByRemote==error);
        else
            assert(error);
        return error;
    }
    
    Err SocketBase::receive(UInt16& received, void* buffer, UInt16 bufferLength, Int32 timeout, UInt16 flags)
    {
        assert(socket_!=0);
        Err error=errNone;
        Int16 result=NetLibReceive(netLib_, socket_, buffer, bufferLength, flags, 0, 0, timeout, &error);
        if (result>=0)
        {
            assert(!error);
            received+=result;
        }
        else
            assert(error);
        return error;
    }
        
        
    Err Socket::connect(const SocketAddress& address, Int32 timeout)
    {
        assert(socket_!=0);
        Err error=errNone;
        SocketAddress::ConstNetSocketAddrProxy proxy=address.getNetSocketAddrProxy();
        const NetSocketAddrType* addr=proxy.first;
        UInt16 addrLen=proxy.second;
        Int16 result=NetLibSocketConnect(netLib_, socket_, const_cast<NetSocketAddrType*>(addr), addrLen, timeout, &error);
        if (result<0)
            assert(error);
        return error;
    }

}