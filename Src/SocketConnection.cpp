#include "SocketConnection.hpp"

namespace ArsLexis 
{

    void SocketConnectionManager::addConnection(SocketConnection& connection)
    {
        NetSocketRef ref=connection.socket_;
        assert(ref>=connections_.size() || 0==connections_[ref]);
        if (ref>=connections_.size())
            connections_.resize(ref+1);
        connections_[ref]=&connection;
    }

    void SocketConnectionManager::registerEvent(SocketConnection& connection, SocketSelector::EventType event)
    {
        NetSocketRef ref=connection.socket_;
        assert(connections_.size()>ref && &connection==connections_[ref]);
        selector_.registerSocket(connection.socket_, event);
    }
    
    void SocketConnectionManager::unregisterEvents(SocketConnection& connection)
    {
        NetSocketRef ref=connection.socket_;
        assert(connections_.size()>ref && &connection==connections_[ref]);
        selector_.unregisterSocket(connection.socket_, SocketSelector::eventRead);
        selector_.unregisterSocket(connection.socket_, SocketSelector::eventWrite);
        selector_.unregisterSocket(connection.socket_, SocketSelector::eventException);
    }
    
    void SocketConnectionManager::removeConnection(SocketConnection& connection)
    {
        NetSocketRef ref=connection.socket_;
        assert(connections_.size()>ref && &connection==connections_[ref]);
        unregisterEvents(connection);
        connections_[ref]=0;
    }
    
    SocketConnectionManager::SocketConnectionManager(NetLibrary& netLib):
        netLib_(netLib),
        selector_(netLib, false)
    {}
    
    SocketConnectionManager::~SocketConnectionManager()
    {
        UInt16 connCount=connections_.size();
        for (UInt16 i=0; i<connCount; ++i)
            if (connections_[i])
                connections_[i]->abortConnection();
    }


    Err SocketConnectionManager::manageConnectionEvents(Int32 timeout)
    {
        Err error=selector_.select(timeout);
        if (!error)
        {
            UInt16 eventsCount=selector_.eventsCount();
            assert(eventsCount>0);
            UInt16 connCount=connections_.size();
            for (UInt16 i=0; i<connCount; ++i)
            {
                SocketConnection* conn=connections_[i];
                if (conn)
                {
                    if (selector_.checkSocketEvent(conn->socket_, SocketSelector::eventRead))
                    {
                        unregisterEvents(*conn);
                        conn->notifyReadable();
                        if (!--eventsCount)
                            break;
                    }
                    if (selector_.checkSocketEvent(conn->socket_, SocketSelector::eventWrite))
                    {
                        unregisterEvents(*conn);
                        conn->notifyWritable();
                        if (!--eventsCount)
                            break;
                    } 
                    //! @bug There's another bug in PalmOS that causes us to receive exception notification, even though we didn't register for it.
                    //! Well, that's not a real problem, because not registering for exceptions should be considered a bug anyway...
                    if (selector_.checkSocketEvent(conn->socket_, SocketSelector::eventException))
                    {
                        unregisterEvents(*conn);
                        conn->notifyException();
                        if (!--eventsCount)
                            break;
                    }                        
                }
            }
        }
        return error;
    }
    
    
    SocketConnection::SocketConnection(SocketConnectionManager& manager):
        manager_(manager),
        transferTimeout_(evtWaitForever),
        address_(0),
        log_("SocketConnection"),
        socket_(manager.netLib_)
    {
    }
    
    SocketConnection::~SocketConnection()
    {
        NetSocketRef ref=socket_;
        if (ref)
            manager_.removeConnection(*this);
    }

    void SocketConnection::open()
    {
        assert(address_!=0);
        Err error=socket_.open();
        if (error)
        {
            log()<<"open(): unable to open socket, "<<error;
            handleError(error);
            return;
        }

        manager_.addConnection(*this);

        //error=socket_.setNonBlocking();
        if (error)
        {
            log()<<"open(), can't setNonBlocking(), "<<error;
            handleError(error);
            return;
        }

        error=socket_.connect(*address_, transferTimeout());
        if (netErrWouldBlock==error)
        {
            // log()<<"open(), got netErrWouldBlock from connect(), changed to errNone";
            error=errNone;
        }

        if (netErrSocketBusy==error)
        {
            log()<<"open(), got netErrSocketBusy from connect(), changed to errNone";
            error=errNone;
        }

        if (error)
        {
            log()<<"open(): can't connect(), "<<error;
            handleError(error);
            return;
        }

        registerEvent(SocketSelector::eventException);
        registerEvent(SocketSelector::eventWrite);

        assert(errNone==error);
    }

    void SocketConnection::notifyException()
    {
        Err error=getSocketErrorStatus();
        if (errNone==error)
        {
            log()<<"notifyException(): getSocketErrorStatus() returned errNone.";
            assert(false);
            error=netErrTimeout;
        }
        else
            log()<<"notifyException(): getSocketErrorStatus() returned error, "<<error;
        handleError(error);
    }

    // devnote: seems to return non-PalmOS error codes
    // e.g. 10061 is WSAECONNREFUSED (http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winsock/winsock/windows_sockets_error_codes_2.asp)
    // those seem to be defined in Core\System\Unix\sys_errno.h but without the 10000 (0x2710) prefix 
    Err SocketConnection::getSocketErrorStatus() const
    {
        NetSocketRef socketRef=socket_;
        assert(socketRef!=0);
        int     status=0;
        UInt16  size=sizeof(status);
        //! @bug PalmOS <5 returns error==netErrParamErr here always, although everything is done according to documentation.
        //! Nevertheless status is also filled in these cases and seems right...
        Err error=socket_.getOption(netSocketOptLevelSocket, netSocketOptSockErrorStatus, &status, size);
        if (error)
        {
            log()<<"getSocketErrorStatus(): unable to query socket option, "<<error;
            return error;
        }
        if (status)
        {
            log()<<"getSocketErrorStatus(): error status, "<<(Err)status;
        }            
        return (Err)status;
    }

}
