#include <SimpleSocketConnection.hpp>
#include <SysUtils.hpp>

namespace ArsLexis
{

    SimpleSocketConnection::SimpleSocketConnection(SocketConnectionManager& manager):
        SocketConnection(manager),
        maxResponseSize_(32768),
        sending_(true),
        chunkSize_(512),
        requestBytesSent_(0),
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
        uint_t requestSize=request_.size();
        uint_t requestLeft=requestSize-requestBytesSent_;
        if (requestLeft>chunkSize_)
            requestLeft=chunkSize_;
        uint_t dataSize=0;
        status_t error=socket().send(dataSize, request_.data()+requestBytesSent_, requestLeft, transferTimeout());
        if (errNone==error || netErrWouldBlock==error)
        {
            registerEvent(SocketSelector::eventRead);
            requestBytesSent_+=dataSize;
            if (requestBytesSent_ == requestSize)
            {
                sending_ = false;
/*                
                error=socket().shutdown(netSocketDirOutput);
                if (error)
                    log().debug()<<_T("notifyWritable(): Socket::shutdown() returned error: ")<<error;
*/                    
            }
            else
                registerEvent(SocketSelector::eventWrite);                
            error=notifyProgress();
        }
        else
            log().error()<<_T("notifyWritable(): Socket::send() returned error: ")<<error;

        return error;        
    }
    
    status_t SimpleSocketConnection::notifyReadable()
    {
        char *  newDataBuf;
        uint_t  dataSize = 0;
        uint_t  curResponseSize;

        if (sending_) 
            log().debug()<<_T("notifyReadable(): called while sending data, probably some connection error occured");
        status_t status = errNone;
        status_t error = getSocketErrorStatus(status);
        if (errNone == error)
            error = status;
        else 
        {
            log().info()<<_T("notifyReadable(): getSocketErrorStatus() returned error (ignored): ")<<error;
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
        error = socket().receive(dataSize, newDataBuf, chunkSize_, transferTimeout());
        if (errNone!=error)
            goto Exit;

        totalReceived_ += dataSize;
        assert(dataSize<=chunkSize_);

        // TODO: I don't like this resize hack. It's not very safe to do things
        // this way. Also, I don't know how resize is implemented - it might
        // call realloc() for every resize which would be inefficient and probably
        // lead to memory fragmentation. It would be better to do reserver()/resize()
        // than resize()/resize()
        // maybe we should just use temporary buffer for receive() or manually
        // control the whole response buffer as char* instead of abusing NarrowString
        resizeResponse(curResponseSize+dataSize);
        //if (chunkSize_ != dataSize)
        if (0==dataSize)
        {   
            log().info()<<_T("notifyReadable(): dataSize != chunkSize_ (server shut socket down?)");
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
            log().error()<<_T("notifyReadable(): Socket::receive() returned error: ")<<error;
        return error;
    }
    
    status_t SimpleSocketConnection::notifyFinished()
    {
        status_t error=socket().shutdown(netSocketDirBoth);
        if (error)
            log().debug()<<_T("notifyFinished(): Socket::shutdown() returned error: ")<<error;
        return error;
    }
    
    status_t SimpleSocketConnection::notifyProgress()
    {return errNone;}

    SimpleSocketConnection::~SimpleSocketConnection()
    {}
 
    status_t SimpleSocketConnection::open()
    {
        status_t error=SocketConnection::open();
        if (!error)
        {
            uint_t size;
            status_t ignore=socket().getMaxTcpSegmentSize(size);
            if (errNone==ignore)
            {
                if (size<=2048)
                {
                    log().info()<<_T("SimpleSocketConnection::open(): setting chunkSize to ")<<size;
                    setChunkSize(size);
                }
            }
            else
                log().info()<<_T("SimpleSocketConnection::open(): error (ignored) while querying maxTcpSegmentSize: ")<<ignore;
        }
        return error;
    }
        
}
