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
	response_(NULL),
	responseLen_(0),
    totalReceived_(0),
	chunk_(NULL)
{}

status_t SimpleSocketConnection::notifyWritable()
{
    assert(sending_);
    ulong_t toSend = requestLenLeft_;
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
    ulong_t  curResponseSize;

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
    if (errNone != error)
        goto Exit;

    curResponseSize = responseLen_;
    if (curResponseSize >= maxResponseSize_ - chunkSize_)
    {
        error = errResponseTooLong;
        goto Exit;
    }

	if (NULL == chunk_)
		chunk_ = (char*)malloc(chunkSize_);
	if (NULL == chunk_)
	{
		error = memErrNotEnoughSpace;
		goto Exit;
	}

    dataSize = 0;
    error = socket().receive(dataSize, chunk_, chunkSize_, transferTimeout());
    if (errNone != error)
        goto Exit;

    totalReceived_ += dataSize;
    assert(dataSize <= chunkSize_);

	response_ = StrAppend(response_, responseLen_, chunk_, dataSize);
	if (NULL == response_)
	{
		error = memErrNotEnoughSpace;
		goto Exit;
	}
    responseLen_ += dataSize;

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
        free(request_);

	if (NULL != response_)
		free(response_);

	if (NULL != chunk_)
		free(chunk_);
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

status_t SimpleSocketConnection::setRequest(const char* request, ulong_t requestSize)
{
    assert( NULL == request_ );
    char* newRequest = (char*)malloc(requestSize);
    if (NULL == newRequest)
        return memErrNotEnoughSpace;

    memmove(newRequest, request, requestSize);
    setRequestOwn(newRequest, requestSize);
    return errNone;
}

