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
    // yes, this is always a ascii 8-bit string, not char_t since 
    char *       request_;
    uint_t       requestLenLeft_;
    uint_t       requestSent_;

    NarrowString response_;
    uint_t       maxResponseSize_;
    uint_t       chunkSize_;
    ulong_t      totalReceived_;
    
    bool         sending_;
    
    status_t     resizeResponse(NarrowString::size_type size);

protected:

    status_t open();
    
    status_t notifyWritable();
    
    status_t notifyReadable();
    
    virtual status_t notifyFinished();
    
    virtual status_t notifyProgress();

    // set request to send to a copy of request
    void setRequestCopy(const char *request, uint_t requestSize)
    {
        assert( NULL == request_ );
        char *newRequest = (char*)malloc(requestSize);
        if (NULL == newRequest)
        {
            // TODO: need to signal an error to the caller
            return;
        }
        memmove(newRequest, request, requestSize);
        setRequestOwn(newRequest, requestSize);
    }

    void setRequest(const NarrowString& request)
    {
        setRequestCopy(request.c_str(), request.size());
    }
    
    void setResponse(const NarrowString& response)
    {response_=response;}

    NarrowString& response()
    {return response_;}
    
public:

    SimpleSocketConnection(SocketConnectionManager& manager);

    // for perf, set request to a given data and take ownership of it
    // i.e. the caller should not free this data since we will
    void setRequestOwn(char *request, uint_t requestSize)
    {
        request_ = request;
        requestLenLeft_ = requestSize;
    }
    
    void setMaxResponseSize(uint_t size)
    {maxResponseSize_=size;}
    
    void setChunkSize(uint_t size)
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
