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
//        typedef std::vector<SocketConnection*, Allocator<SocketConnection*> > Connections_t;
        typedef std::vector<SocketConnection*> Connections_t;
        Connections_t connections_;
        
        void addConnection(SocketConnection& connection);
    
        void registerEvent(SocketConnection& connection, SocketSelector::EventType event);
        
        void unregisterEvents(SocketConnection& connection);
        
        void removeConnection(SocketConnection& connection);
        
    public:
    
        SocketConnectionManager(NetLibrary& netLib);

        ~SocketConnectionManager();
        
        Boolean active() const
        {return selector_.active();}
        
        Err manageConnectionEvents(Int32 timeout=evtWaitForever);
        
        NetLibrary& netLibrary()
        {return netLib_;}
        
        friend class SocketConnection;
    };

    class SocketConnection
    {
        SocketConnectionManager& manager_;
        Int32 transferTimeout_;
        const SocketAddress* address_;
        
    protected:

        Socket socket_;
    
        Err socketStatus() const;
    
        virtual void notifyWritable()
        {}
        
        virtual void notifyReadable()
        {}
        
        virtual void notifyException();
        
        virtual void abortConnection()
        {delete this;}
        
        SocketConnection(SocketConnectionManager& manager);
        
    public:
    
        virtual void handleError(Err error)
        {abortConnection();}
        
        void setTransferTimeout(Int32 timeout)
        {transferTimeout_=timeout;}
        
        Int32 transferTimeout() const
        {return transferTimeout_;}
        
        void setAddress(const SocketAddress& address)
        {address_=&address;}

        virtual void open();
        
        void registerEvent(SocketSelector::EventType event)
        {
            manager_.registerEvent(*this, event);
        }
        
        virtual ~SocketConnection();
        
        SocketConnectionManager& manager()
        {return manager_;}
        
        friend class SocketConnectionManager;
    };

}

#endif