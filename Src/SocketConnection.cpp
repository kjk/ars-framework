#include <SysUtils.hpp>
#include <SocketConnection.hpp>
#include <DeviceInfo.hpp>
#include <Resolver.hpp>

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
#ifdef _WIN32
    stop_(false),
    event_(NULL),
#endif         
    connectionsCount_(0)
{}

SocketConnectionManager::~SocketConnectionManager()
{
    abortConnections();
#ifdef _WIN32
    if (NULL != event_)
        CloseHandle(event_);
#endif      
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

void SocketConnectionManager::compactConnections()
{
    int countAfter = connectionsCount_;
    int curPos = 0;
    for (int i=0; i<connectionsCount_; i++)
    {
        if (NULL != connections_[i])
        {
            if (i!=curPos)
            {
                connections_[curPos] = connections_[i];
                curPos++;
            }
        }
        else
        {
            --countAfter;
        }
    }
    connectionsCount_ = countAfter;
}
    
bool SocketConnectionManager::manageFinishedConnections()
{ 
    LockGuard guard(lock_);
    bool fDeletedConnection=false;
    for (int i=0; i<connectionsCount_; i++)
    {
        SocketConnection* conn=connections_[i];
        if (SocketConnection::stateFinished==conn->state())
        {
            delete conn;
            connections_[i]=NULL;
            fDeletedConnection=true;
        }
    }
    if (fDeletedConnection)
    {
        compactConnections();
        if (0==connectionsCount_)
            netLib_.close();
    }
    return fDeletedConnection;
}

bool SocketConnectionManager::manageUnresolvedConnections()
{
    LockGuard guard(lock_);
    for (int i=0; i<connectionsCount_; i++)
    {
        SocketConnection* conn = connections_[i];
        if (SocketConnection::stateUnresolved==conn->state())
        {
            status_t error=errNone;
            if (netLib_.closed())
                error=openNetLib();
            if (!error)
            {
                guard.release();
                error=conn->resolve();
            }  
            if (error)
            {
                guard.acquire();
                conn->handleError(error);
                delete conn;
                connections_[i]=NULL;
                compactConnections();
            }
            return true;                            
        }
    }
    return false;
}
    
bool SocketConnectionManager::manageUnopenedConnections()
{
    LockGuard guard(lock_);
    for (int i=0; i<connectionsCount_; i++)
    {
        SocketConnection* conn = connections_[i];
        if (SocketConnection::stateUnopened==conn->state())
        {
            guard.release();
            status_t error=conn->open();
            if (error)
            {
                guard.acquire();
                conn->handleError(error);
                delete conn;
                connections_[i]=NULL;
                compactConnections();
            }
            return true;
        }
    }
    return false;
}

status_t SocketConnectionManager::handleTimeout(long span)
{
    LockGuard guard(lock_);
    for (int i = 0; i < connectionsCount_; ++i)
    {
        SocketConnection* conn = connections_[i];
        if (evtWaitForever == conn->transferTimeout_)
            continue;
            
        conn->currentTimeout_ += span;
        if (conn->currentTimeout_ > conn->transferTimeout_)
        {
            conn->handleError(netErrTimeout);
            delete conn;
            connections_[i] = NULL;
        }
    }
    compactConnections();
    return errNone;
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
    if (netErrTimeout == error)
        return handleTimeout(timeout);

    if (errNone != error)
        return error;
        
    for (int i=0; i<connectionsCount_; i++)
    {
        status_t connErr=errNone;
        SocketConnection* conn = connections_[i];
        assert(SocketConnection::stateOpened==conn->state());
        bool done=false;
        if (selector_.checkSocketEvent(conn->socket(), SocketSelector::eventException))
        {
            unregisterEvents(*conn);
            conn->resetTimeout();
            connErr=conn->notifyException();
            done=true;
        }                        
        else if (selector_.checkSocketEvent(conn->socket_, SocketSelector::eventWrite))
        {
            unregisterEvents(*conn);
            conn->resetTimeout();
            connErr=conn->notifyWritable();
            done=true;
        } 
        else if (selector_.checkSocketEvent(conn->socket_, SocketSelector::eventRead))
        {
            unregisterEvents(*conn);
            conn->resetTimeout();
            connErr=conn->notifyReadable();
            done=true;
        }
        if (connErr)
        {
            conn->handleError(connErr);
            delete conn;
            connections_[i]=NULL;
        }
        if (done)
            break;
    }
    compactConnections();
    return errNone;
}

#ifdef _PALM_OS 

bool SocketConnectionManager::checkEvent(EventType& event)
{
    if (EvtEventAvail())
    {
        EvtGetEvent(&event, 0);
        return true;
    }
    return false;
}


status_t SocketConnectionManager::waitForEvent(EventType& event, long timeout)
{
    while (true)
    {    
        if (checkEvent(event))
            return errNone;
        if (manageFinishedConnections() && checkEvent(event))
            return errNone;
        if (manageUnresolvedConnections() && checkEvent(event))
            return errNone;
        if (manageUnopenedConnections() && checkEvent(event))
            return errNone;
        status_t error=selector_.selectWithInputEvents(timeout);
        if (netErrTimeout == error)
			return handleTimeout(timeout);

        if (errNone != error)
            return error;
            
        for (int i=0; i<connectionsCount_; i++)
        {
            status_t connErr=errNone;
            SocketConnection* conn = connections_[i];
            assert(SocketConnection::stateOpened==conn->state());
            if (selector_.checkSocketEvent(conn->socket(), SocketSelector::eventException))
            {
                unregisterEvents(*conn);
				conn->resetTimeout();
                connErr = conn->notifyException();
            }                        
            else if (selector_.checkSocketEvent(conn->socket_, SocketSelector::eventWrite))
            {
                unregisterEvents(*conn);
				conn->resetTimeout();
                connErr = conn->notifyWritable();
            } 
            else if (selector_.checkSocketEvent(conn->socket_, SocketSelector::eventRead))
            {
                unregisterEvents(*conn);
				conn->resetTimeout();
                connErr = conn->notifyReadable();
            }
            if (connErr)
            {
                conn->handleError(connErr);
                delete conn;
                connections_[i] = NULL;
            }
        }
        compactConnections();
        if (0 == connectionsCount_)
            break;
        if (selector_.isInputEvent())
            break;
    }
    EvtGetEvent(&event, timeout);
    return errNone;
}
#endif    

#ifdef _WIN32
status_t SocketConnectionManager::waitForMessage(MSG& msg, long timeout)
{
    while (true)
    {    
		if (0 == connectionsCount_)
			break;
			
        if (peekMessage(msg))
            return errNone;

        if (manageFinishedConnections() && peekMessage(msg))
            return errNone;

		if (0 == connectionsCount_)
			break;

        if (manageUnresolvedConnections() && peekMessage(msg))
            return errNone;

		if (0 == connectionsCount_)
			break;

        if (manageUnopenedConnections() && peekMessage(msg))
            return errNone;
            
		if (0 == connectionsCount_)
			break;

        status_t error=selector_.select(timeout);
        if (netErrTimeout == error)
			return handleTimeout(timeout);

        if (errNone != error)
            return error;
            
        for (int i=0; i<connectionsCount_; i++)
        {
            status_t connErr=errNone;
            SocketConnection* conn = connections_[i];
            assert(SocketConnection::stateOpened==conn->state());
            if (selector_.checkSocketEvent(conn->socket(), SocketSelector::eventException))
            {
                unregisterEvents(*conn);
				conn->resetTimeout();
                connErr = conn->notifyException();
            }                        
            else if (selector_.checkSocketEvent(conn->socket_, SocketSelector::eventWrite))
            {
                unregisterEvents(*conn);
				conn->resetTimeout();
                connErr = conn->notifyWritable();
            } 
            else if (selector_.checkSocketEvent(conn->socket_, SocketSelector::eventRead))
            {
                unregisterEvents(*conn);
				conn->resetTimeout();
                connErr = conn->notifyReadable();
            }
            if (connErr)
            {
                conn->handleError(connErr);
                delete conn;
                connections_[i] = NULL;
            }
        }
        compactConnections();
    }
    BOOL res = GetMessage(&msg, NULL, 0, 0);
	if (-1 == res)
		return GetLastError();
	
    return errNone;
}

bool SocketConnectionManager::peekMessage(MSG& msg)
{
	return FALSE != PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
}
#endif

void SocketConnectionManager::abortConnections() 
{
    LockGuard g(lock_);
    for(int i=0; i<connectionsCount_; i++)
    {
        SocketConnection* conn = connections_[i];
        conn->abortConnection();
        delete conn;
        connections_[i] = NULL;
    }
    connectionsCount_ = 0;
}

SocketConnection::SocketConnection(SocketConnectionManager& manager):
    manager_(manager),
    state_(stateUnresolved),
    transferTimeout_(evtWaitForever),
    socket_(manager.netLib_),
    currentTimeout_(0)
{
}

SocketConnection::~SocketConnection()
{
    abortConnection();
}

void SocketConnection::abortConnection() 
{
    manager_.unregisterEvents(*this);
    setState(stateFinished);
}

status_t SocketConnection::open()
{
    assert(stateUnopened==state());
    status_t error=socket_.open();
    if (error)
    {
        LogStrUlong(eLogDebug, _T("open(): unable to open socket, "), error);
        return error;
    }

#ifdef _WIN32
    error = socket_.setNonBlocking();
    if (error)
    {
        LogStrUlong(eLogInfo, _T("open(), can't setNonBlocking(), "), error);
        return error;
    }
#endif    


    error=socket_.connect(address_, transferTimeout());
    if (netErrWouldBlock==error)
    {
        Log(eLogInfo, _T("open(), got netErrWouldBlock from connect(), changed to errNone"), true);
        error=errNone;
    }

    if (netErrSocketBusy==error)
    {
        Log(eLogInfo, _T("open(), got netErrSocketBusy from connect(), changed to errNone"), true);
        error=errNone;
    }
    
    if (!error)
    {
        setState(stateOpened);
        registerEvent(SocketSelector::eventWrite);
        registerEvent(SocketSelector::eventRead); // For purpose of detecting connection close
    }
    return error;
}

status_t SocketConnection::notifyException()
{
    status_t status=errNone;
    status_t error=getSocketErrorStatus(status);
    if (errNone==error)
        LogStrUlong(eLogDebug, _T("notifyException(): getSocketErrorStatus() returned status: "), status);
    return status;
}

// devnote: seems to return non-PalmOS error codes
// e.g. 10061 is WSAECONNREFUSED (http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winsock/winsock/windows_sockets_error_codes_2.asp)
// those seem to be defined in Core\System\Unix\sys_errno.h but without the 10000 (0x2710) prefix 
status_t SocketConnection::getSocketErrorStatus(status_t& out) const
{
#if defined(_PALM_OS)
    if (6 == romVersionMajor())
        return netErrUnimplemented;
#endif

    NativeSocket_t socketRef=socket_;
    assert(socketRef!=0);
    int     status=0;
    uint_t  size=sizeof(status);
    //! @bug PalmOS <5 returns error==netErrParamErr here always, although everything is done according to documentation.
    //! Nevertheless status is also filled in these cases and seems right...
#ifdef NDEBUG        
    status_t error = socket_.getOption(socketOptLevelSocket, socketOptSockErrorStatus, &status, size);
#else        
    status_t error = errNone;
# ifdef _PALM_OS
    if (!(underSimulator() && isTreo600())) // These fatal alerts on Treo600 sim really piss me off.
        error = socket_.getOption(socketOptLevelSocket, socketOptSockErrorStatus, &status, size);
# endif
#endif
        
    if (error)
        LogStrUlong(eLogInfo, _T("getSocketErrorStatus(): unable to query socket option, error: "), error);
    else 
        out=status;
    out=status;
    return error;
}

status_t SocketConnection::resolve()
{
    assert(stateUnresolved==state());
    status_t error = ::resolve(address_, manager_.netLib_, serverAddress, 0, transferTimeout());
    if (errNone==error)
        setState(stateUnopened);
    return error;
}


status_t SocketConnection::enqueue()
{
    assert(stateUnresolved==state());
    return manager_.enqueueConnection(*this);
}


status_t SocketConnection::notifyWritable()
{
    return errNone;
}
    
status_t SocketConnection::notifyReadable()
{
    return errNone;
}
    
void SocketConnection::handleError(status_t)
{
    abortConnection();
}

status_t SocketConnectionManager::enqueueConnection(SocketConnection& conn)
{
    LockGuard guard(lock_);
    if (maxConnections == connectionsCount_)
        return netErrUnimplemented;

    connections_[connectionsCount_++] = &conn;
#ifdef _WIN32
    if (NULL != event_)
        SetEvent(event_);
#endif    
    return errNone;
}

#ifdef _WIN32
status_t SocketConnectionManager::runManagerThread()
{
    status_t err = errNone;
    if (NULL == event_ && NULL == (event_ = CreateEvent(NULL, FALSE, FALSE, NULL)))
    { 
        err = GetLastError();  
        return err;
    }
    while (true)
    {
        LockGuard guard(lock_);
        ulong_t count = connectionsCount_;
        guard.release();
        if (0 == count)
        {
            DWORD res = WaitForSingleObject(event_, INFINITE);
            if (WAIT_FAILED == res)
            {
                err = GetLastError();
                return err; 
            }   
        }
        if (stop_)
            break;
        
        while (manageFinishedConnections());
        
        if (stop_)
            break;

        while (manageUnresolvedConnections());
        
        if (stop_)
            break;

        while (manageUnopenedConnections());
        
        if (stop_)
            break;
            
        guard.acquire();
        if (0 == connectionsCount_)
        {
            guard.release();
            continue;
        }
            
        err = selector_.select(20);
        guard.release();
        
        if (stop_)
            break;
            
        if (netErrTimeout == err)
        {
            handleTimeout(20);
            continue;
        }
            
        if (errNone != err)
            return err;
  
        if (stop_)
            break;
        
        guard.acquire();
        for (int i=0; i<connectionsCount_; i++)
        {
            status_t connErr = errNone;
            SocketConnection* conn = connections_[i];
            assert(SocketConnection::stateOpened == conn->state());
            if (selector_.checkSocketEvent(conn->socket(), SocketSelector::eventException))
            {
                unregisterEvents(*conn);
                conn->resetTimeout();
                connErr=conn->notifyException();
            }                        
            else if (selector_.checkSocketEvent(conn->socket_, SocketSelector::eventWrite))
            {
                unregisterEvents(*conn);
                conn->resetTimeout();
                connErr=conn->notifyWritable();
            } 
            else if (selector_.checkSocketEvent(conn->socket_, SocketSelector::eventRead))
            {
                unregisterEvents(*conn);
                conn->resetTimeout();
                connErr=conn->notifyReadable();
            }
            if (connErr)
            {
                conn->handleError(connErr);
                delete conn;
                connections_[i]=NULL;
            }
        }
        compactConnections();
        guard.release();
    }
    return errNone; 
}

DWORD SocketConnectionManager::managerThreadProc(LPVOID param)
{
    SocketConnectionManager* manager = (SocketConnectionManager*)param;
    assert(NULL != manager);
    return manager->runManagerThread();  
}

HANDLE SocketConnectionManager::startManagerThread()
{
    return CreateThread(NULL, 0, &SocketConnectionManager::managerThreadProc, this, 0, NULL);
}

void SocketConnectionManager::stop()
{
    assert(NULL != event_);
    stop_ = true;
    SetEvent(event_);  
}

#endif

