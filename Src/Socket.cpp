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
        socket_(invalidSocket)
    {}
    
    void SocketBase::close() 
    {
        assert(isOpen());        
        status_t error;
        int result=netLib_.socketClose(socket_, evtWaitForever, error);
        if (-1==result || errNone!=error)
            log().error()<<_T("close(): NetLibSocketClose() returned error: ")<<error;
        socket_=invalidSocket;
    }
    
    SocketBase::~SocketBase()
    {
        if (isOpen())
            close();
    }
    
    status_t SocketBase::open(NativeSockAddrFamily_t domain, NativeSocketType_t type, short protocol, long timeout)
    {
        assert(!isOpen());
        status_t error;
        socket_= netLib_.socketOpen(domain, type, protocol, timeout, error);
        assert(error || isOpen());
        return error;
    }
    
    status_t SocketBase::shutdown(short direction, long timeout)
    {
        assert(isOpen());
        status_t error=errNone;
        int result=netLib_.socketShutdown(socket_, direction, timeout, error);
        if (-1==result)
            assert(error);
        else
            assert(!error);
        if (error)
            log().debug()<<_T("shutdown(): NetLibSocketShutdown() returned error: ")<<error;
        return error;
    }
    
    status_t SocketBase::send(uint_t& sent, const void* buffer, uint_t bufferLength, long timeout, uint_t flags,  const SocketAddress* address)
    {
        assert(isOpen());
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

    // read data from the socket until we get an error or fill out the buffer
    // or the other side closes the socket
    status_t SocketBase::receive(uint_t& received, void* buffer, uint_t bufferLength, long timeout, uint_t flags)
    {
        assert(isOpen());
        status_t error = errNone;
        int result;

        uint_t bytesRead = 0;
        uint_t bytesLeft = bufferLength;

        while (true)
        {
            result = netLib_.socketReceive(socket_, buffer, bytesLeft, flags, 0, 0, timeout, error);
            if (errNone != error)
                break;
            assert(result >= 0);
            if (0==result)
                break;

            bytesRead += result;
            bytesLeft -= result;
            // TODO: if you remove break, we'll try to fill out the buffer until
            // we get all the data or the connection is closed
            // currently it break iPedia (we get malfromedResponse for an
            // unknown reason)
            break;
            if (0==bytesLeft)
                break;
        }
        received = bytesRead;
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
        assert(isOpen());
        if (netFeatureUnimplemented==level || netFeatureUnimplemented==option)
            return netErrUnimplemented;
        status_t error=errNone;
        int result=netLib_.socketOptionSet(socket_, level, option, const_cast<void*>(optionValue), valueLength, timeout, error);
        if (-1==result)
            assert(error);
        else
            assert(!error);
        if (error)            
            log().debug()<<_T("setOption(): NetLibSocketOptionSet() returned error: ")<<error;
        return error;
    }
    
    status_t SocketBase::getOption(uint_t level, uint_t option, void* optionValue, uint_t& valueLength, long timeout) const
    {
        assert(isOpen());
        if (netFeatureUnimplemented==level || netFeatureUnimplemented==option)
            return netErrUnimplemented;
        status_t error=errNone;
        int result=netLib_.socketOptionGet(socket_, level, option, optionValue, valueLength, timeout, error);
        if (-1==result)
            assert(error);
        else
            assert(!error);
        if (error)            
            log().warning()<<_T("getOption(): NetLibSocketOptionGet() returned error: ")<<error;
        return error;
    }
        
    status_t Socket::connect(const SocketAddress& address, long timeout)
    {
        assert(isOpen());
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
        status_t error=setOption(socketOptLevelSocket, socketOptSockLinger, &linger, sizeof(linger));
        return error;
    }
    
    status_t SocketBase::getMaxTcpSegmentSize(uint_t& size)
    {
        uint_t len=sizeof(size);
        status_t error=getOption(socketOptLevelTCP, socketOptTCPMaxSeg, &size, len);
        assert(sizeof(size)==len);
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
        recalculateWidth();
        
        for (ushort_t i=0; i<eventTypesCount_; ++i)
            outputFDs_[i] = inputFDs_[i];

        status_t error=errNone;
        // Seems NetLibSelect() is really badly screwed in PalmOS - see strange error behaviour in the following if-else...
        short eventsCount=netLib_.select( width_+1, &outputFDs_[eventRead], &outputFDs_[eventWrite], &outputFDs_[eventException], timeout, error);
        if (-1 == eventsCount)
        {
            if (!error)
                error=netErrTimeout;
            eventsCount_=0;
        }
        else
        {
            assert(!error);
            eventsCount_ = eventsCount;
            if (0==eventsCount_)
                error=netErrTimeout;
        }            
        return error;
    }

#ifdef _PALM_OS    
    status_t SocketSelector::selectWithInputEvents(long timeout)
    {
        netFDSet(sysFileDescStdIn, &inputFDs_[eventRead]);
        recalculateWidth();
        
        for (ushort_t i=0; i<eventTypesCount_; ++i)
            outputFDs_[i] = inputFDs_[i];

        status_t error=errNone;
        // Seems NetLibSelect() is really badly screwed in PalmOS - see strange error behaviour in the following if-else...
        short eventsCount=netLib_.select( width_+1, &outputFDs_[eventRead], &outputFDs_[eventWrite], &outputFDs_[eventException], timeout, error);
        if (-1 == eventsCount)
        {
            if (!error)
                error=netErrTimeout;
            eventsCount_=0;
        }
        else
        {
            assert(!error);
            eventsCount_ = eventsCount;
            if (0==eventsCount_)
                error=netErrTimeout;
        }            
        return error;
    }
#endif

    void SocketSelector::recalculateWidth()
    {
        width_=1;
        for (ushort_t i = sizeof(NativeFDSet_t) * 8-1; i>0; --i)
            if (netFDIsSet(i, &inputFDs_[eventRead]) ||
                netFDIsSet(i, &inputFDs_[eventWrite]) ||
                netFDIsSet(i, &inputFDs_[eventException]))
            {
                width_ = i+1;
                break;
            }                
    }
    
}