#ifndef __ARSLEXIS_SIMPLE_SOCKET_CONNECTION_HPP__
#define __ARSLEXIS_SIMPLE_SOCKET_CONNECTION_HPP__

#include <SocketConnection.hpp>
#include <BaseTypes.hpp>

#ifdef _MSC_VER
# pragma warning(disable: 4068)
#endif

namespace ArsLexis 
{
    
    class SimpleSocketConnection: public SocketConnection
    {
        NarrowString request_;
        NarrowString response_;
        uint_t maxResponseSize_;
        uint_t chunkSize_;
        ulong_t totalReceived_;
        
        bool sending_;
        uint_t requestBytesSent_;
        
        status_t resizeResponse(NarrowString::size_type size);
        
    protected:
    
        status_t open();
        
        status_t notifyWritable();
        
        status_t notifyReadable();
        
        virtual status_t notifyFinished();
        
        virtual status_t notifyProgress();
        
        void setRequest(const NarrowString& request)
        {request_=request;}
        
        void setResponse(const NarrowString& response)
        {response_=response;}

        NarrowString& response()
        {return response_;}
        
    public:

        SimpleSocketConnection(SocketConnectionManager& manager);
        
        void setMaxResponseSize(uint_t size)
        {maxResponseSize_=size;}
        
        void setChunkSize(uint_t size)
        {chunkSize_=size;}
        
        bool sending() const
        {return sending_;}
        
        ~SimpleSocketConnection();
    
    };
    
}

#endif