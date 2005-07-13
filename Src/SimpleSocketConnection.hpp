#ifndef __ARSLEXIS_SIMPLE_SOCKET_CONNECTION_HPP__
#define __ARSLEXIS_SIMPLE_SOCKET_CONNECTION_HPP__

#include <SocketConnection.hpp>
#include <BaseTypes.hpp>
#include <Text.hpp>

#ifdef _MSC_VER
# pragma warning(disable: 4068)
#endif
    
class SimpleSocketConnection: public SocketConnection
{
    char*       request_;
    ulong_t       requestLenLeft_;
    ulong_t       requestSent_;

    ulong_t       maxResponseSize_;
    ulong_t       chunkSize_;
    ulong_t      totalReceived_;
    
    bool         sending_;
	char*		chunk_;

protected:

    char*		response_;
	ulong_t		responseLen_;
    
protected:

    status_t open();
    
    status_t notifyWritable();
    
    status_t notifyReadable();
    
    virtual status_t notifyFinished();
    
    virtual status_t notifyProgress();

    // set request to send to a copy of request
    status_t setRequest(const char* request, ulong_t requestSize);

	/*
    status_t setRequest(const NarrowString& request)
    {
        return setRequestCopy(request.data(), request.size());
    }
    
    void setResponse(const NarrowString& response)
    {response_ = response;}

    NarrowString& response()
    {return response_;}
	*/
    
public:

    SimpleSocketConnection(SocketConnectionManager& manager);

    // for perf, set request to a given data and take ownership of it
    // i.e. the caller should not free this data since we will
    void setRequestOwn(char *request, uint_t requestSize)
    {
        request_ = request;
        requestLenLeft_ = requestSize;
    }
    
    void setMaxResponseSize(ulong_t size)
    {maxResponseSize_=size;}
    
    void setChunkSize(ulong_t size)
    {chunkSize_=size;}
    
    bool sending() const
    {return sending_;}
    
    ~SimpleSocketConnection();

    bool fRequestExists()
    {
        if (NULL == request_)
            return false;
        return true;
    }

};

#endif
