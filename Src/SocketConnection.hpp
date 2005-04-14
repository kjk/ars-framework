#ifndef __ARSLEXIS_SOCKET_CONNECTION_HPP__
#define __ARSLEXIS_SOCKET_CONNECTION_HPP__

#include <ErrBase.h>
#include <NetLibrary.hpp>
#include <Logging.hpp>
#include <SocketAddress.hpp>
#include <Socket.hpp>

#define MAX_CONNECTIONS 5

class SocketConnection;

class SocketConnectionManager: private NonCopyable
{
    NetLibrary      netLib_;
    SocketSelector  selector_;

    int                 connectionsCount_;
    SocketConnection *  connections_[MAX_CONNECTIONS];

    void registerEvent(SocketConnection& connection, SocketSelector::EventType event);

    void unregisterEvents(SocketConnection& connection);

    NetLibrary& netLibrary()
    {return netLib_;}

    status_t openNetLib();

    void compactConnections();

    bool manageFinishedConnections();
    bool manageUnresolvedConnections();
    bool manageUnopenedConnections();

	status_t handleTimeout(long span);


public:

    SocketConnectionManager();

    ~SocketConnectionManager();

    bool active() const
    {return 0!=connectionsCount_;}

    status_t manageConnectionEvents(long timeout = evtWaitForever);
    
#ifdef _PALM_OS        
    status_t waitForEvent(EventType& event, long timeout = evtWaitForever);
#endif
    
    void abortConnections();

    friend class SocketConnection;
#ifdef _PALM_OS        
private:
    static bool checkEvent(EventType& event);
#endif        
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
    State                    state_;
    long                     transferTimeout_;
    SocketAddress            address_;
    Socket                   socket_;
	long					currentTimeout_;

protected:

    Socket& socket() 
    {return socket_;}

    const Socket& socket() const
    {return socket_;}

    status_t getSocketErrorStatus(status_t& out) const;

    virtual status_t resolve();

    virtual status_t notifyWritable();

    virtual status_t notifyReadable();

    virtual status_t notifyException();

    virtual void abortConnection();

    virtual status_t open();

    void setState(State state)
    {state_=state;}

    virtual void handleError(status_t);

    void registerEvent(SocketSelector::EventType event)
    {manager_.registerEvent(*this, event);}

    SocketConnection(SocketConnectionManager& manager);

	void resetTimeout() {currentTimeout_ = 0;}

public:

    const char_t*  serverAddress;

    State state() const
    {return state_;}

    enum SockError
    {
        errResponseTooLong=sockConnErrorClass,  // 0x8200
        errResponseMalformed,  // 0x8201
        errNetLibUnavailable,  // 0x8202
        errFirstAvailable      // 0x8203
    };

    virtual status_t enqueue();

    void setTransferTimeout(long timeout)
    {
        transferTimeout_=timeout;
    }

    long transferTimeout() const
    {return transferTimeout_;}

    virtual ~SocketConnection();

    friend class SocketConnectionManager;
};


#endif
