#ifndef __ARSLEXIS_SIMPLE_SOCKET_CONNECTION_HPP__
#define __ARSLEXIS_SIMPLE_SOCKET_CONNECTION_HPP__

#include <SocketConnection.hpp>
#include <BaseTypes.hpp>

namespace ArsLexis 
{
    
    class SimpleSocketConnection: public SocketConnection
    {
        String request_;
        String response_;
        UInt16 maxResponseSize_;
        UInt16 chunkSize_;
        
        bool sending_;
        UInt16 requestBytesSent_;
        
    protected:
        
        Err notifyWritable();
        
        Err notifyReadable();
        
        virtual Err notifyFinished()
        {return errNone;}
        
        virtual Err notifyProgress()
        {return errNone;}
        
        void setRequest(const String& request)
        {request_=request;}
        
        String& response()
        {return response_;}
        
    public:

        SimpleSocketConnection(SocketConnectionManager& manager);
        
        void setMaxResponseSize(UInt16 size)
        {maxResponseSize_=size;}
        
        void setChunkSize(UInt16 size)
        {chunkSize_=size;}
        
        bool sending() const
        {return sending_;}
    
    };
    
}

#endif