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
        String request_;
        String response_;
        uint_t maxResponseSize_;
        uint_t chunkSize_;
        
        bool sending_;
        uint_t requestBytesSent_;
        
        status_t resizeResponse(String::size_type size);
        
    protected:
        
        status_t notifyWritable();
        
        status_t notifyReadable();
        
        virtual status_t notifyFinished();
        
        virtual status_t notifyProgress();
        
        void setRequest(const String& request)
        {request_=request;}
        
        String& response()
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