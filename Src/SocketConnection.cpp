#include <SocketConnection.hpp>
#include <algorithm>



namespace ArsLexis 
{

    void SocketConnectionManager::registerEvent(SocketConnection& connection, SocketSelector::EventType event)
    {
        selector_.registerSocket(connection.socket(), event);
    }
    
    void SocketConnectionManager::unregisterEvents(SocketConnection& connection)
    {
        Socket& socket=connection.socket();
        selector_.unregisterSocket(socket, SocketSelector::eventRead);
        selector_.unregisterSocket(socket, SocketSelector::eventWrite);
        selector_.unregisterSocket(socket, SocketSelector::eventException);
    }
    
    SocketConnectionManager::SocketConnectionManager():
        selector_(netLib_, false),
        resolver_(netLib_)
    {}
    
    SocketConnectionManager::~SocketConnectionManager()
    {
        std::for_each(connections_.begin(), connections_.end(), ObjectDeleter<SocketConnection>());
    }
    
    status_t SocketConnectionManager::openNetLib()
    {
        assert(netLib_.closed());
        uint_t ifError;
        status_t error=netLib_.initialize(ifError);
        if (error || ifError)
            error=SocketConnection::errNetLibUnavailable;
        return error;
    }

    bool SocketConnectionManager::manageFinishedConnections()
    {
        bool done=false;
        Connections_t::iterator it=connections_.begin();
        while (it!=connections_.end())
        {
            if (SocketConnection::stateFinished==(*it)->state())
            {
                SocketConnection* conn=*it;
                delete conn;
                Connections_t::iterator next=it;
                ++next;
                connections_.erase(it);
                done=true;
                it=next;
            }
            else
                ++it;
        }
        if (done)
        {
            if (connections_.empty())
                netLib_.close();
        }
        return done;
    }

    bool SocketConnectionManager::manageUnresolvedConnections()
    {
        Connections_t::iterator end=connections_.end();
        for (Connections_t::iterator it=connections_.begin(); it!=end; ++it)
            if (SocketConnection::stateUnresolved==(*it)->state())
            {
                SocketConnection* conn=*it;
                status_t error=errNone;
                if (netLib_.closed())
                    error=openNetLib();
                if (!error)                
                    error=conn->resolve(resolver_);
                if (error)
                {
                    conn->handleError(error);
                    delete conn;
                    connections_.erase(it);
                }
                return true;                            
            }
        return false;
    }
        
    bool SocketConnectionManager::manageUnopenedConnections()
    {
        Connections_t::iterator end=connections_.end();
        for (Connections_t::iterator it=connections_.begin(); it!=end; ++it)
            if (SocketConnection::stateUnopened==(*it)->state())
            {
                SocketConnection* conn=*it;
                status_t error=conn->open();
                if (error)
                {
                    conn->handleError(error);
                    delete conn;
                    connections_.erase(it);
                }
                return true;
            }
        return false;
    }
        
    status_t SocketConnectionManager::manageConnectionEvents(long timeout)
    {
        if (manageFinishedConnections())
            return errNone;
            
        if (manageUnresolvedConnections())            
            return errNone;
            
        if (manageUnopenedConnections())
            return errNone;
            
        status_t error=selector_.select(timeout);
        if (error)
            return error;
            
        Connections_t::iterator end=connections_.end();
        for (Connections_t::iterator it=connections_.begin(); it!=end; ++it)
        {
            status_t connErr=errNone;
            SocketConnection* conn=*it;
            assert(SocketConnection::stateOpened==conn->state());
            bool done=false;
            if (selector_.checkSocketEvent(conn->socket(), SocketSelector::eventException))
            {
                unregisterEvents(*conn);
                connErr=conn->notifyException();
                done=true;
            }                        
            else if (selector_.checkSocketEvent(conn->socket_, SocketSelector::eventWrite))
            {
                unregisterEvents(*conn);
                connErr=conn->notifyWritable();
                done=true;
            } 
            else if (selector_.checkSocketEvent(conn->socket_, SocketSelector::eventRead))
            {
                unregisterEvents(*conn);
                connErr=conn->notifyReadable();
                done=true;
                
            }
            if (connErr)
            {
                conn->handleError(connErr);
                delete conn;
                connections_.erase(it);
            }
            if (done)
                break;
        }
        return errNone;
    }
    
    
    SocketConnection::SocketConnection(SocketConnectionManager& manager):
        manager_(manager),
        state_(stateUnresolved),
        transferTimeout_(evtWaitForever),
        log_(_T("SocketConnection")),
        socket_(manager.netLib_)
    {
    }
    
    SocketConnection::~SocketConnection()
    {}
    
    void SocketConnection::abortConnection() 
    {
        setState(stateFinished);
    }

    status_t SocketConnection::open()
    {
        assert(stateUnopened==state());
        status_t error=socket_.open();
        if (error)
        {
            log().debug()<<_T("open(): unable to open socket, ")<<error;
            return error;
        }

        //error=socket_.setNonBlocking();
        if (error)
        {
            log().info()<<_T("open(), can't setNonBlocking(), ")<<error;
            return error;
        }

        error=socket_.connect(address_, transferTimeout());
        if (netErrWouldBlock==error)
        {
            log().info()<<_T("open(), got netErrWouldBlock from connect(), changed to errNone");
            error=errNone;
        }

        if (netErrSocketBusy==error)
        {
            log().info()<<_T("open(), got netErrSocketBusy from connect(), changed to errNone");
            error=errNone;
        }
        
        if (!error)
        {
            setState(stateOpened);
            registerEvent(SocketSelector::eventException);
            registerEvent(SocketSelector::eventWrite);
        }
        return error;
    }

    status_t SocketConnection::notifyException()
    {
        status_t error=getSocketErrorStatus();
        if (errNone==error)
        {
            log().debug()<<_T("notifyException(): getSocketErrorStatus() returned errNone.");
            assert(false);
            error=netErrTimeout;
        }
        else
            log().debug()<<_T("notifyException(): getSocketErrorStatus() returned error, ")<<error;
            
        return error;
    }

    // devnote: seems to return non-PalmOS error codes
    // e.g. 10061 is WSAECONNREFUSED (http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winsock/winsock/windows_sockets_error_codes_2.asp)
    // those seem to be defined in Core\System\Unix\sys_errno.h but without the 10000 (0x2710) prefix 
    status_t SocketConnection::getSocketErrorStatus() const
    {
        NativeSocket_t socketRef=socket_;
        assert(socketRef!=0);
        int     status=0;
        uint_t  size=sizeof(status);
        //! @bug PalmOS <5 returns error==netErrParamErr here always, although everything is done according to documentation.
        //! Nevertheless status is also filled in these cases and seems right...
        status_t error=socket_.getOption(socketOptLevelSocket, socketOptSockErrorStatus, &status, size);
        if (error)
        {
            log().error()<<_T("getSocketErrorStatus(): unable to query socket option, ")<<error;
            return error;
        }
        if (status)
        {
            log().debug()<<_T("getSocketErrorStatus(): error status, ")<<(status_t)status;
        }            
        return (status_t)status;
    }
    
    status_t SocketConnection::resolve(Resolver& resolver)
    {
        assert(stateUnresolved==state());
        status_t error=resolver.resolve(address_, addressString_, 0, transferTimeout());
        if (!error)
            setState(stateUnopened);
        return error;
    }
    
    status_t SocketConnection::enqueue()
    {
        assert(stateUnresolved==state());
        manager_.connections_.push_back(this);
        return errNone;
    }

}
