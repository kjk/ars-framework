#include <SimpleSocketConnection.hpp>
#include <SysUtils.hpp>
#include <DeviceInfo.hpp>

SimpleSocketConnection::SimpleSocketConnection(SocketConnectionManager& manager):
    SocketConnection(manager),
    maxResponseSize_(32768),
    sending_(true),
    chunkSize_(576),
    request_(NULL),
    requestLenLeft_(0),
    requestSent_(0),
    totalReceived_(0)
{}

status_t SimpleSocketConnection::resizeResponse(NarrowString::size_type size)
{
    status_t error=errNone;
    ErrTry {
        response_.resize(size);
    }
    ErrCatch(ex) {
        error=ex;
    } ErrEndCatch
    return error;
}

status_t SimpleSocketConnection::notifyWritable()
{
    assert(sending_);
    uint_t toSend = requestLenLeft_;
    if (requestLenLeft_ > chunkSize_)
        toSend = chunkSize_;
    uint_t dataSize = 0;
    status_t error = socket().send(dataSize, request_ + requestSent_, requestLenLeft_, transferTimeout());
    if (errNone==error || netErrWouldBlock==error)
    {
        registerEvent(SocketSelector::eventRead);
        requestSent_ += dataSize;
        assert( requestLenLeft_ >= dataSize );
        requestLenLeft_ -= dataSize;
        if (0 == requestLenLeft_)
        {
            sending_ = false;
            error = socket().shutdown(netSocketDirOutput);
            if (error)
                LogStrUlong(eLogDebug, _T("notifyWritable(): Socket::shutdown() returned error: "), error);
        }
        else
            registerEvent(SocketSelector::eventWrite);                
        error = notifyProgress();
    }
    else
        LogStrUlong(eLogError, _T("notifyWritable(): Socket::send() returned error: "), error);

    return error;        
}

status_t SimpleSocketConnection::notifyReadable()
{
    uint_t dataSize = 0;
    char* newDataBuf;
    uint_t  curResponseSize;

    if (sending_) 
        Log(eLogDebug, _T("notifyReadable(): called while sending data, probably some connection error occured"), true);

    status_t status = errNone;
    status_t error = getSocketErrorStatus(status);
    if (errNone == error)
        error = status;
    else 
    {
        LogStrUlong(eLogInfo, _T("notifyReadable(): getSocketErrorStatus() returned error (ignored): "), error);
        error = errNone;
    }
    if (errNone!=error)
        goto Exit;

    curResponseSize = response_.size();
    if (curResponseSize >= maxResponseSize_-chunkSize_)
    {
        error = errResponseTooLong;
        goto Exit;
    }

    error = resizeResponse(curResponseSize+chunkSize_);
    if (errNone!=error)
        goto Exit;

    newDataBuf = (char*)&response_[curResponseSize];
    dataSize = 0;
    error = socket().receive(dataSize, newDataBuf, chunkSize_, transferTimeout());
    if (errNone!=error)
        goto Exit;

    totalReceived_ += dataSize;
    assert(dataSize<=chunkSize_);
    
    resizeResponse(curResponseSize+dataSize);
    //if (chunkSize_ != dataSize)
    if (0 == dataSize)
    {   
        // Log(eLogInfo, _T("notifyReadable(): 0 == dataSize (server shut socket down?)"), true);
        // TODO: if we use chunkSize_ != dataSize condition, we also need
        // to notifyProgress();
        //error=notifyProgress();
        //if (error)
        //    goto Exit;
        error = notifyFinished();
        abortConnection();
    }
    else
    {
        registerEvent(SocketSelector::eventRead);
        error = notifyProgress();
    }
Exit:
    if (errNone!=error)
        LogStrUlong(eLogError, _T("notifyReadable(): Socket::receive() returned error: "), error);
    return error;
}

status_t SimpleSocketConnection::notifyFinished()
{
    status_t error=socket().shutdown(netSocketDirInput);
    if (error)
        LogStrUlong(eLogDebug, _T("notifyFinished(): Socket::shutdown() returned error: "), error);
    return error;
}

status_t SimpleSocketConnection::notifyProgress()
{return errNone;}

SimpleSocketConnection::~SimpleSocketConnection()
{
    if (NULL != request_)
    {
        free(request_);
    }
}

status_t SimpleSocketConnection::open()
{
    status_t error = SocketConnection::open();
    if (!error)
    {
        uint_t size = chunkSize_;
#ifdef NDEBUG
        status_t ignore = socket().getMaxTcpSegmentSize(size);
#else
        status_t ignore = errNone;
# ifdef _PALM_OS
        if (!(underSimulator() && isTreo600())) // These fatal alerts on Treo600 sim really piss me off.
            ignore = socket().getMaxTcpSegmentSize(size);
# endif
#endif                
        if (errNone == ignore)
        {
            if (size <= 2048)
            {
                // LogStrUlong(eLogInfo, _T("SimpleSocketConnection::open(): setting chunkSize to "), size);
                setChunkSize(size);
            }
        }
        else
            LogStrUlong(eLogInfo, _T("SimpleSocketConnection::open(): error (ignored) while querying maxTcpSegmentSize: "), ignore);
    }
    return error;
}

