#ifndef __ARSLEXIS_SIMPLE_SOCKET_CONNECTION_HPP__
#define __ARSLEXIS_SIMPLE_SOCKET_CONNECTION_HPP__

#include "SocketConnection.hpp"

namespace ArsLexis 
{
    
    class SimpleSocketConnection: public SocketConnection
    {
        String request_;
        String response_;
        UInt16 maxResponseSize_;
        UInt16 chunkSize_;
        
        bool sending_;
        UInt32 transferTimeout_;
        UInt16 requestBytesSent_;
        
    protected:
        
        void notifyWritable();
        
        void notifyReadable();
        
        virtual void handleError(Err error)
        {abortConnection();}
        
        virtual void finalize()
        {abortConnection();}
        
        void notifyException();
        
        virtual void reportProgress()
        {}
        
    public:

        SimpleSocketConnection(SocketConnectionManager& manager, const String& request);
        
        ~SimpleSocketConnection();
        
        const String& response() const
        {return response_;}

        void setMaxResponseSize(UInt16 size)
        {maxResponseSize_=size;}
        
        void setTransferTimeout(Int32 timeout)
        {transferTimeout_=timeout;}
        
        void setChunkSize(UInt16 size)
        {chunkSize_=size;}
        
        bool sending() const
        {return sending_;}
    
    };
}

#endif