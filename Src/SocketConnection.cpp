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
                    // There's another bug in PalmOS that causes us to receive exception notification, even though we didn't register for it.
                    // Well, that's not a real problem, because not registering for exceptions should be considered a bug anyway...
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
        socket_(manager.netLib_)
    {
    }
    
    SocketConnection::~SocketConnection()
    {
        manager_.removeConnection(*this);
    }
    
    Err SocketConnection::open(const SocketAddress& address, Int32 timeout)
    {
        Err error=socket_.open();
        if (!error)
        {
            manager_.addConnection(*this);
            Boolean flag=true;
            error=socket_.setOption(netSocketOptLevelSocket, netSocketOptSockNonBlocking, &flag, sizeof(flag));
            if (!error)
                error=socket_.connect(address, timeout);
        }
        return error;
    }

    void SocketConnection::notifyException()
    {
        Err error=socketStatus();
        if (error!=errNone)
            abortConnection();
    }

    Err SocketConnection::socketStatus() const
    {
        NetSocketRef socketRef=socket_;
        assert(socketRef!=0);
        Err status=errNone;
        UInt16 size=sizeof(status);
        // PalmOS <5 returns error==netErrParamErr here always, although everything is done according to documentation.
        // Nevertheless status is also filled in these cases and seems right...
        Err error=socket_.getOption(netSocketOptLevelSocket, netSocketOptSockErrorStatus, &status, size);
        return status;
    }

}