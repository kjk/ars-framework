#ifndef __ARSLEXIS_SOCKET_CONNECTION_HPP__
#define __ARSLEXIS_SOCKET_CONNECTION_HPP__

#include "Socket.hpp"
#include <vector>

namespace ArsLexis
{
    class SocketConnection;

    class SocketConnectionManager
    {
        NetLibrary& netLib_;
        SocketSelector selector_;
        typedef std::vector<SocketConnection*, Allocator<SocketConnection*> > Connections_t;
        Connections_t connections_;
        
        void addConnection(SocketConnection& connection);
    
        void registerEvent(SocketConnection& connection, SocketSelector::EventType event);
        
        void unregisterEvent(SocketConnection& connection, SocketSelector::EventType event);
        
        void removeConnection(SocketConnection& connection);
        
    public:
    
        SocketConnectionManager(NetLibrary& netLib);

        ~SocketConnectionManager();
        
        Boolean connectionsAvailable() const
        {return !connections_.empty();}
        
        Err runUntilEvent();
        
        friend class SocketConnection;
    };

    class SocketConnection
    {
        SocketConnectionManager& manager_;
        Socket socket_;
        
    protected:
    
//        Err errorStatus();
    
        virtual void notifyWritable()
        {}
        
        virtual void notifyReadable()
        {}
        
        virtual void notifyException();
        
        virtual void abortConnection()
        {delete this;}
        
    public:
        
        void registerEvent(SocketSelector::EventType event)
        {
            manager_.registerEvent(*this, event);
        }
        
        SocketConnection(SocketConnectionManager& manager);
        
        virtual ~SocketConnection();
        
        Err open(const SocketAddress& address, Int32 timeout=evtWaitForever);
        
        friend class SocketConnectionManager;
    };

}

#endif