#include <Socket.hpp>
#include <SocketAddress.hpp>
#include <NetLibrary.hpp>

namespace ArsLexis
{

#pragma mark -
#pragma mark SocketBase

    SocketBase::SocketBase(NetLibrary& netLib):
        log_("SocketBase"),
        netLib_(netLib),
        socket_(0)
    {}
    
    SocketBase::~SocketBase()
    {
        if (socket_!=0)
        {
            status_t error=netLib_.socketClose(socket_, evtWaitForever, &error);
            if (error)
                log().error()<<"~SocketBase(): NetLibSocketClose() returned error, "<<error;
        }
    }
    
    status_t SocketBase::open(NativeSockAddrFamily_t domain, NativeSocketType_t type, short protocol, long timeout)
    {
        assert(!socket_);
        status_t error=errNone;
        socket_= netLib_.socketOpen(domain, type, protocol, timeout, &error);
        assert(error || (socket_!=0));
        return error;
    }
    
    status_t SocketBase::shutdown(short direction, long timeout)
    {
        assert(socket_!=0);
        status_t error=errNone;
        short result=netLib_.socketShutdown(socket_, direction, timeout, &error);
        if (-1==result)
            assert(error);
        else
            assert(!error);
        if (error)
            log()<<"shutdown(): NetLibSocketShutdown() returned error, "<<error;
        return error;
    }
    
    status_t SocketBase::send(ushort_t& sent, const void* buffer, ushort_t bufferLength, long timeout, ushort_t flags, const SocketAddress* address)
    {
        assert(socket_!=0);
        status_t error=errNone;
        const  NativeSocketAddr_t* addr=0;
        ushort_t addrLen=0;
        if (address)
        {
            addr=*address;
            addrLen=address->size();
        }
        short result=netLib_.socketSend(socket_, const_cast<void*>(buffer), bufferLength, flags, 
            const_cast<NativeSocketAddr_t*>(addr), addrLen, timeout, &error);
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
    
    status_t SocketBase::receive(ushort_t& received, void* buffer, ushort_t bufferLength, long timeout, ushort_t flags)
    {
        assert(socket_!=0);
        status_t error=errNone;
        
        short result=netLib_.socketReceive(socket_, buffer, bufferLength, flags, 0, 0, timeout, &error);
        if (result>=0)
        {
            assert(!error);
            received+=result;
        }
        else
            assert(error);
        return error;
    }

    status_t SocketBase::setNonBlocking(bool value)
    {
        Boolean flag=value;
        status_t error=setOption(SocketOptLevelSocket_c, SocketOptSockNonBlocking_c, &flag, sizeof(flag));
        return error;
    } 

    status_t SocketBase::setOption(ushort_t level, ushort_t option, void* optionValue, ushort_t valueLength, long timeout)
    {
        assert(socket_!=0);
        status_t error=errNone;
        short result=netLib_.socketOptionSet( socket_, level, option, optionValue, valueLength, timeout, &error);
        if (-1==result)
            assert(error);
        else
            assert(!error);
        if (error)            
            log()<<"setOption(): NetLibSocketOptionSet() returned error, "<<error;
        return error;
    }
    
    status_t SocketBase::getOption(ushort_t level, ushort_t option, void* optionValue, ushort_t& valueLength, long timeout) const
    {
        assert(socket_!=0);
        status_t error=errNone;
        short result=netLib_.socketOptionGet(socket_, level, option, optionValue, &valueLength, timeout, &error);
        if (-1==result)
            assert(error);
        else
            assert(!error);
        if (error)            
            log().warning()<<"getOption(): NetLibSocketOptionGet() returned error, "<<error;
        return error;
    }
        
    status_t Socket::connect(const SocketAddress& address, long timeout)
    {
        assert(socket_!=0);
        status_t error=errNone;
        const NativeSocketAddr_t* addr=address;
        ushort_t addrLen=address.size();
        short result=NetLibSocketConnect(netLib_, socket_, const_cast<NativeSocketAddr_t*>(addr), addrLen, timeout, &error);
        if (-1==result)
            assert(error);
        else
            assert(!error);
        return error;
    }

    status_t SocketBase::setLinger(const CommonSocketLinger_t& linger)
    {
        assert(socket_!=0);
        status_t error=setOption(SocketOptLevelSocket_c, SocketOptSockLinger_c,&( (const_cast<CommonSocketLinger_t*>(&linger))->native), sizeof(linger));
        return error;
    }
    
/*    
    status_t SocketBase::getLinger(CommonSocketLinger_t& linger) const
    {
        assert(socket_!=0);
        status_t error=getOption(netSocketOptLevelSocket, netSocketOptSockLinger, &linger, sizeof(linger);
        return error;
    }
*/    
    
#pragma mark -
#pragma mark SocketSelector

    SocketSelector::SocketSelector(NetLibrary& netLib, bool catchStandardEvents):
        netLib_(netLib),
        width_(0),
        eventsCount_(0)
    {
        for (ushort_t i=0; i<eventTypesCount_; ++i)
        {
            netFDZero(&inputFDs_[i]);
            netFDZero(&outputFDs_[i]);
        }
        if (catchStandardEvents)
            netFDSet(sysFileDescStdIn, &inputFDs_[eventRead]);
        recalculateWidth();
    }
    
    status_t SocketSelector::select(long timeout)
    {
        for (ushort_t i=0; i<eventTypesCount_; ++i)
            outputFDs_[i]=inputFDs_[i];

        status_t error=errNone;
        // Seems NetLibSelect() is really badly screwed in PalmOS - see strange error behaviour in the following if-else...
        short eventsCount=netLib_.select( width_+1, &outputFDs_[eventRead], &outputFDs_[eventWrite], &outputFDs_[eventException], timeout, &error);
        if (-1==eventsCount)
        {
            if (!error)
                error=netErrTimeout;
            eventsCount_=0;
        }
        else
        {
            assert(!error);
            eventsCount_=eventsCount;
            if (0==eventsCount_)
                error=netErrTimeout;
        }            
        return error;
    }
    
    void SocketSelector::recalculateWidth()
    {
        width_=1;
        for (ushort_t i=sizeof(NativeFDSet_t)*8-1; i>0; --i)
            if (netFDIsSet(i, &inputFDs_[eventRead]) ||
                netFDIsSet(i, &inputFDs_[eventWrite]) ||
                netFDIsSet(i, &inputFDs_[eventException]))
            {
                width_=i+1;
                break;
            }                
    }
    
                
}