#ifndef __ARSLEXIS_SOCKET_CONNECTION_HPP__
#define __ARSLEXIS_SOCKET_CONNECTION_HPP__

#include <Socket.hpp>
#include <ErrBase.h>
#include <Logging.hpp>
#include <SocketAddress.hpp>
#include <Resolver.hpp>
#include <NetLibrary.hpp>
#include <list>

namespace ArsLexis
{

    class SocketConnection;

    class SocketConnectionManager: private NonCopyable
    {
        NetLibrary netLib_;
        SocketSelector selector_;
        Resolver resolver_;
        
        typedef std::list<SocketConnection*> Connections_t;
        Connections_t connections_;
        
        void registerEvent(SocketConnection& connection, SocketSelector::EventType event);
        
        void unregisterEvents(SocketConnection& connection);
        
        NetLibrary& netLibrary()
        {return netLib_;}
        
        Err openNetLib();
        
        bool manageFinishedConnections();
        bool manageUnresolvedConnections();
        bool manageUnopenedConnections();
        
    public:
    
        SocketConnectionManager();

        ~SocketConnectionManager();
        
        bool active() const
        {return !connections_.empty();}
        
        Err manageConnectionEvents(Int32 timeout=evtWaitForever);
        
        friend class SocketConnection;
    };

    class SocketConnection: private NonCopyable
    {
    public:
        enum State
        {
            stateUnresolved,
            stateUnopened,
            stateOpened,
            stateFinished
        };

    private:
        SocketConnectionManager& manager_;
        State state_;
        Int32 transferTimeout_;
        ArsLexis::String addressString_;
        SocketAddress address_;
        mutable ChildLogger log_;
        Socket socket_;
        
    protected:
    
        Socket& socket() 
        {return socket_;}
        
        const Socket& socket() const
        {return socket_;}

        Err getSocketErrorStatus() const;
        
        virtual Err resolve(Resolver& resolver);
    
        virtual Err notifyWritable()
        {return errNone;}
        
        virtual Err notifyReadable()
        {return errNone;}
        
        virtual Err notifyException();
        
        virtual void abortConnection();
        
        virtual Err open();
        
        void setState(State state)
        {state_=state;}
        
        virtual void handleError(Err)
        {abortConnection();}
        
        void registerEvent(SocketSelector::EventType event)
        {manager_.registerEvent(*this, event);}
        
        SocketConnection(SocketConnectionManager& manager);
        
    public:

        State state() const
        {return state_;}
        
        Logger& log() const
        {return log_;}
    
        enum Error
        {
            errResponseTooLong=sockConnErrorClass,
            errResponseMalformed,
            errNetLibUnavailable,
            errFirstAvailable
        };
        
        virtual Err enqueue();
        
        void setAddress(const String& address)
        {addressString_=address;}
    
        void setTransferTimeout(Int32 timeout)
        {
            transferTimeout_=timeout;
        }
        
        Int32 transferTimeout() const
        {return transferTimeout_;}
        
        virtual ~SocketConnection();
        
        friend class SocketConnectionManager;
    };

}

#endif