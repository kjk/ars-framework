#include "SimpleSocketConnection.hpp"

namespace ArsLexis
{

    SimpleSocketConnection::SimpleSocketConnection(SocketConnectionManager& manager):
        SocketConnection(manager),
        maxResponseSize_(32768),
        sending_(true),
        chunkSize_(256),
        requestBytesSent_(0)
    {
    }

    SimpleSocketConnection::~SimpleSocketConnection()
    {
    }

    void SimpleSocketConnection::notifyWritable()
    {
        assert(sending_);
        UInt16 requestSize=request_.size();
        UInt16 requestLeft=requestSize-requestBytesSent_;
        if (requestLeft>chunkSize_)
            requestLeft=chunkSize_;
        UInt16 dataSize=0;
        Err error=socket_.send(dataSize, request_.data()+requestBytesSent_, requestLeft, transferTimeout());
        if (errNone==error || netErrWouldBlock==error)
        {
            registerEvent(SocketSelector::eventException);
            requestBytesSent_+=dataSize;
            if (requestBytesSent_==requestSize)
            {
                sending_=false;
                registerEvent(SocketSelector::eventRead);
                error=socket_.shutdown(netSocketDirOutput);
            }
            else
                registerEvent(SocketSelector::eventWrite);                
            reportProgress();
        }
        else
            handleError(error);
    }
    
    void SimpleSocketConnection::notifyReadable()
    {
        assert(!sending_);
        UInt16 dataSize=0;
        UInt16 responseSize=response_.size();
        if (responseSize<maxResponseSize_-chunkSize_)
        {
            response_.resize(responseSize+chunkSize_);
            Err error=socket_.receive(dataSize, const_cast<char*>(response_.data())+responseSize, chunkSize_, transferTimeout());
            if (!error)
            {
                assert(dataSize<=chunkSize_);
                response_.resize(responseSize+dataSize);
                if (0==dataSize)
                    finalize();
                else
                {
                    registerEvent(SocketSelector::eventException);
                    registerEvent(SocketSelector::eventRead);
                    reportProgress();
                }
            }
            else
                handleError(error);
        }
        else
            handleError(netErrBufTooSmall);                
    }
        
    void SimpleSocketConnection::notifyException()
    {
        Err error=socketStatus();
        assert(error!=errNone);
        handleError(error);
    }

}