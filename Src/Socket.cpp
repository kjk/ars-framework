#include <Socket.hpp>
#include <SocketAddress.hpp>
#include <NetLibrary.hpp>

namespace ArsLexis
{

#pragma mark -
#pragma mark SocketBase

    SocketBase::SocketBase(NetLibrary& netLib):
        log_(_T("SocketBase")),
        netLib_(netLib),
        socket_(0)
    {}
    
    SocketBase::~SocketBase()
    {
        if (socket_!=0)
        {
            status_t error;
            int result=netLib_.socketClose(socket_, evtWaitForever, error);
            if (-1==result || error)
                log().error()<<_T("~SocketBase(): NetLibSocketClose() returned error, ")<<error;
        }
    }
    
    status_t SocketBase::open(NativeSockAddrFamily_t domain, NativeSocketType_t type, short protocol, long timeout)
    {
        assert(!socket_);
        status_t error;
        socket_= netLib_.socketOpen(domain, type, protocol, timeout, error);
        //Shouldn't be here socket_!=-1
        assert(error || (socket_!=0));
        return error;
    }
    
    status_t SocketBase::shutdown(short direction, long timeout)
    {
        assert(socket_!=0);
        status_t error=errNone;
        int result=netLib_.socketShutdown(socket_, direction, timeout, error);
        if (-1==result)
            assert(error);
        else
            assert(!error);
        if (error)
            log().debug()<<_T("shutdown(): NetLibSocketShutdown() returned error, ")<<error;
        return error;
    }
    
    status_t SocketBase::send(uint_t& sent, const void* buffer, uint_t bufferLength, long timeout, uint_t flags,  const SocketAddress* address)
    {
        assert(socket_!=0);
        status_t error=errNone;
        const SocketAddr* addr=0;
        uint_t addrLen=0;
        if (address)
        {
            addr=*address;
            addrLen=address->size();
        }
        short result=netLib_.socketSend(socket_, const_cast<void*>(buffer), bufferLength, flags, 
            const_cast<SocketAddr*>(addr), addrLen, timeout, error);
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
    
    status_t SocketBase::receive(uint_t& received, void* buffer, uint_t bufferLength, long timeout, uint_t flags)
    {
        assert(socket_!=0);
        status_t error=errNone;
        int result=netLib_.socketReceive(socket_, buffer, bufferLength, flags, 0, 0, timeout, error);
        if (result>=0)
        {
            assert(!error);
            received+=result;
        }
        else
            assert(error);
        return error;
    }

    /*status_t SocketBase::setNonBlocking(bool value)
    {
        bool flag=value;
        status_t error=setOption(socketOptLevelSocket, socketOptSockNonBlocking, &flag, sizeof(flag));
        return error;
    } */

    status_t SocketBase::setOption(uint_t level, uint_t option, const void* optionValue, uint_t valueLength, long timeout)
    {
        assert(socket_!=0);
        status_t error=errNone;
        int result=netLib_.socketOptionSet(socket_, level, option, const_cast<void*>(optionValue), valueLength, timeout, error);
        if (-1==result)
            assert(error);
        else
            assert(!error);
        if (error)            
            log().debug()<<_T("setOption(): NetLibSocketOptionSet() returned error, ")<<error;
            
        return error;
    }
    
    status_t SocketBase::getOption(uint_t level, uint_t option, void* optionValue, uint_t& valueLength, long timeout) const
    {
        assert(socket_!=0);
        status_t error=errNone;
        int result=netLib_.socketOptionGet(socket_, level, option, optionValue, valueLength, timeout, error);
        if (-1==result)
            assert(error);
        else
            assert(!error);
        if (error)            
            log().warning()<<_T("getOption(): NetLibSocketOptionGet() returned error, ")<<error;
        return error;
    }
        
    status_t Socket::connect(const SocketAddress& address, long timeout)
    {
        assert(socket_!=0);
        status_t error=errNone;
        const SocketAddr* addr=address;
        uint_t addrLen=address.size();
        int result=netLib_.socketConnect(socket_, *addr, addrLen, timeout, error);
        if (-1==result)
            assert(error);
        else
            assert(!error);
        return error;
    }

    status_t SocketBase::setLinger(const SocketLinger& linger)
    {
        assert(socket_!=0);
        status_t error=setOption(socketOptLevelSocket, socketOptSockLinger, &linger, sizeof(linger));
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
        //if (catchStandardEvents)
        //    netFDSet(sysFileDescStdIn, &inputFDs_[eventRead]);
        recalculateWidth();
    }
    
    status_t SocketSelector::select(long timeout)
    {
        for (ushort_t i=0; i<eventTypesCount_; ++i)
            outputFDs_[i]=inputFDs_[i];

        status_t error=errNone;
        // Seems NetLibSelect() is really badly screwed in PalmOS - see strange error behaviour in the following if-else...
        short eventsCount=netLib_.select( width_+1, &outputFDs_[eventRead], &outputFDs_[eventWrite], &outputFDs_[eventException], timeout, error);
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