#include "SimpleSocketConnection.hpp"

namespace ArsLexis
{

    SimpleSocketConnection::SimpleSocketConnection(SocketConnectionManager& manager):
        SocketConnection(manager),
        maxResponseSize_(32768),
        sending_(true),
        transferTimeout_(evtWaitForever),
        chunkSize_(1024),
        requestBytesSent_(0)
    {
    }

    SimpleSocketConnection::~SimpleSocketConnection()
    {
    }

    Err SimpleSocketConnection::open(const SocketAddress& address, const String& request, Int32 timeout)
    {
        request_=request;
        Err error=SocketConnection::open(address, timeout);
        if (!error || netErrWouldBlock==error)
        {
            registerEvent(SocketSelector::eventException);
            registerEvent(SocketSelector::eventWrite);
        }
        return error;
    }
    
    void SimpleSocketConnection::notifyWritable()
    {
        assert(sending_);
        UInt16 requestSize=request_.size();
        UInt16 requestLeft=requestSize-requestBytesSent_;
        if (requestLeft>chunkSize_)
            requestLeft=chunkSize_;
        UInt16 dataSize=0;
        Err error=socket_.send(dataSize, request_.data()+requestBytesSent_, requestLeft, transferTimeout_);
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
        response_.resize(responseSize+chunkSize_);
        Err error=socket_.receive(dataSize, const_cast<char*>(response_.data())+responseSize, chunkSize_, transferTimeout_);
        if (!error)
        {
            assert(dataSize<=chunkSize_);
            if (responseSize+dataSize<maxResponseSize_)
            {
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
                handleError(netErrBufTooSmall);
        }
        else
            handleError(error);
    }
        
    void SimpleSocketConnection::notifyException()
    {
        Err error=socketStatus();
        assert(error!=errNone);
        handleError(error);
    }

}