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
        
        status_t openNetLib();
        
        bool manageFinishedConnections();
        bool manageUnresolvedConnections();
        bool manageUnopenedConnections();
        
    public:
    
        SocketConnectionManager();

        ~SocketConnectionManager();
        
        bool active() const
        {return !connections_.empty();}
        
        status_t manageConnectionEvents(long timeout=evtWaitForever);
        
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
        long transferTimeout_;
        ArsLexis::String addressString_;
        SocketAddress address_;
        mutable ChildLogger log_;
        Socket socket_;
        
    protected:
    
        Socket& socket() 
        {return socket_;}
        
        const Socket& socket() const
        {return socket_;}

        status_t getSocketErrorStatus() const;
        
        virtual status_t resolve(Resolver& resolver);
    
        virtual status_t notifyWritable()
        {return errNone;}
        
        virtual status_t notifyReadable()
        {return errNone;}
        
        virtual status_t notifyException();
        
        virtual void abortConnection();
        
        virtual status_t open();
        
        void setState(State state)
        {state_=state;}
        
        virtual void handleError(status_t)
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
        
        virtual status_t enqueue();
        
        void setAddress(const String& address)
        {addressString_=address;}
    
        void setTransferTimeout(long timeout)
        {
            transferTimeout_=timeout;
        }
        
        long transferTimeout() const
        {return transferTimeout_;}
        
        virtual ~SocketConnection();
        
        friend class SocketConnectionManager;
    };

}

#endif