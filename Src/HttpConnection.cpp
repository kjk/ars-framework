#include <HttpConnection.hpp>
#include <Text.hpp>

#ifdef __MWERKS__
# pragma pcrelconstdata on
# pragma far_code
#endif

namespace ArsLexis {

    namespace {
        
        enum {
            requestMethodLength=8,
            requestMethodsCount=8
        };
        
        typedef char_t RequestMethodStorage_t[requestMethodLength];
        typedef RequestMethodStorage_t RequestMethodsArray_t[requestMethodsCount];
        RequestMethodsArray_t requestMethods=
        {
            _T("OPTIONS"),
            _T("GET"),
            _T("HEAD"),
            _T("POST"),
            _T("PUT"),
            _T("DELETE"),
            _T("TRACE"),
            _T("CONNECT")
        };
        
        const char_t* crLf=_T("\r\n");

    }

    HttpConnection::HttpConnection(SocketConnectionManager& manager):
        SimpleSocketConnection(manager),
        protocolVersionMajor_(1),
        protocolVersionMinor_(1),
        requestMethod_(methodGet),
        uri_(_T("/")),
        insideResponseHeaders_(false),
        insideResponseBody_(false),
        chunkedEncoding_(false),
        skippingInfoResponse_(false),
        bodyContentsAvailable_(false),
        finished_(false),
        chunkedBodyFinished_(false),
        contentLength_(contentLengthUnavailable),
        readContentLength_(0)
    {
        log().setThreshold(Logger::logEverything);
    }        

    HttpConnection::~HttpConnection() 
    {
        std::for_each(requestFields_.begin(), requestFields_.end(), ObjectDeleter<RequestField_t>());
    }

    void HttpConnection::renderRequestLine(String& out) 
    {
        assert(requestMethod_<requestMethodsCount);
        const char_t* method=requestMethods[requestMethod_];
        static const int versionBufferLength=16;
        char_t versionBuffer[versionBufferLength];
        uint_t major=protocolVersionMajor_;
        uint_t minor=protocolVersionMinor_;
        int verLen=StrPrintF(versionBuffer, _T("%hu.%hu"), major, minor);
        out.append(method).append(1, _T(' ')).append(uri_).append(_T(" HTTP/"), 6).append(versionBuffer, verLen).append(crLf);
    }

    void HttpConnection::renderHeaderField(String& out, const RequestField_t& field)
    {
        out.append(field.first).append(_T(": "), 2).append(field.second).append(crLf);
    }

    status_t HttpConnection::commitRequest() 
    {
        String request;
        renderRequestLine(request);
        RequestFields_t::const_iterator end=requestFields_.end();
        for (RequestFields_t::const_iterator it=requestFields_.begin(); it!=end; ++it)
            renderHeaderField(request, *(*it));
        std::for_each(requestFields_.begin(), requestFields_.end(), ObjectDeleter<RequestField_t>());
        requestFields_.clear();
        request.append(crLf);
        if (!messageBody_.empty())
        {
            request.append(messageBody_);
            messageBody_.clear();
        }
        setRequest(request);
        return errNone;
    }

    status_t HttpConnection::handleResponseField(const String& field, const String& value)
    {
        status_t error=errNone;
        if (equalsIgnoreCase(field, _T("Transfer-Encoding")))
        {
            if (equalsIgnoreCase(value, _T("chunked")))
                chunkedEncoding_=true;
            else 
                error=errHttpUnknownTransferEncoding;
        }
        else if (equalsIgnoreCase(field, _T("Content-Length")))
        {
            String::size_type len=value.find_first_of(_T("; \t"));
            if (value.npos==len)
                len=value.length();
            long val;
            error=numericValue(value.data(), value.data()+len, val);
            if (errNone==error)
                contentLength_=val;
            else
                error=errResponseMalformed;
        }
        return error;
    }

    status_t HttpConnection::handleStatusLine(uint_t versionMajor, uint_t versionMinor, uint_t statusCode, const String& reason)
    {
        status_t error=errNone;
        if (statusCode>=100 && statusCode<200)
            skippingInfoResponse_=true;
        else if (statusCode!=200)
            error=errHttpUnsupportedStatusCode;
        return error;
    }
    
    status_t HttpConnection::open() 
    {
        commitRequest();
        return SimpleSocketConnection::open();
    }

    void HttpConnection::addRequestHeader(const String& field, const String& value) 
    {
        requestFields_.push_back(new RequestField_t(field, value));
    }
    
    void HttpConnection::setUri(const String& uri) 
    {
        uri_=uri;
        static const int prefixLength=7;
        if (uri.find(_T("http://"))==0)
        {
            String::size_type end=uri.find(_T("/"), prefixLength);
            String address(uri, prefixLength, end-prefixLength);
            if (address.npos==address.find(_T(':')))
                address.append(_T(":80"), 3);
            setAddress(address);            
        }
    }
    
    status_t HttpConnection::notifyReadable() 
    {
        if (!insideResponseBody_)
            return SimpleSocketConnection::notifyReadable();
        else
            return processResponseBody();
    }

    //! @todo Handle case when field value spans through multiple lines
    bool HttpConnection::nextResponseLine(String& out, bool finish)
    {
        String& resp=response();
        String::size_type pos=resp.find(crLf);
        if (resp.npos==pos && !finish)
            return false;
        out.assign(resp, 0, pos);
        eraseStart(resp, resp.npos==pos?pos:pos+2);
        return true;
    }
    
    status_t HttpConnection::processResponseHeaders(bool finish)
    {
        status_t error=errNone;
        String line;
        bool ready=nextResponseLine(line, finish);
        while (ready && !insideResponseBody_) {
            if (line.empty())
            {
                if (skippingInfoResponse_)
                    skippingInfoResponse_=insideResponseBody_=insideResponseHeaders_=false;
                else
                {
                    if (!chunkedBodyFinished_)
                    {
                        insideResponseBody_=true;
                        insideResponseHeaders_=false;
                    }
                    break;
                }
            }
            else 
            {
                if (!insideResponseHeaders_)
                {
                    error=processStatusLine(line);
                    insideResponseHeaders_=true;
                }
                else
                    error=processHeaderLine(line);
            }
            ready=nextResponseLine(line, finish);
        }
        return error;    
    }
    
    status_t HttpConnection::processStatusLine(const String& line)
    {
        if (line.find(_T("HTTP/"))!=0)
            return errResponseMalformed;
        String::size_type pos0=line.find(_T('.'), 6);
        if (line.npos==pos0)
            return errResponseMalformed;
        long value;
        status_t error=numericValue(&line[5], &line[pos0], value);
        if (error)
            return errResponseMalformed;
        uint_t major=value;
        String::size_type pos1=line.find_first_of(_T(" \t"), pos0+1);
        if (line.npos==pos1)
            return errResponseMalformed;
        error=numericValue(&line[pos0+1],&line[pos1], value);
        if (error)
            return errResponseMalformed;
        uint_t minor=value;
        pos0=line.find_first_not_of(_T(" \t"), pos1);
        if (line.npos==pos0)
            return errResponseMalformed;
        pos1=line.find_first_of(_T(" \t"), pos0);
        if (line.npos==pos1)
            return errResponseMalformed;
        error=numericValue(&line[pos0], &line[pos1], value);
        if (error)
            return errResponseMalformed;
        uint_t statusCode=value;
        pos0=line.find_first_not_of(_T(" \t"), pos1);
        if (line.npos==pos0)
            return errResponseMalformed;
        String reason(line, pos0);
        return handleStatusLine(major, minor, statusCode, reason);
    }

    status_t HttpConnection::processHeaderLine(const String& line)
    {
        String::size_type pos=line.find(_T(':'));
        if (line.npos==pos)
            return errResponseMalformed;
        String field(line, 0, pos);
        String value;
        pos=line.find_first_not_of(_T(" \t"), pos+1);
        if (line.npos!=pos)
            value.assign(line, pos, line.npos);
        return handleResponseField(field, value);
    }
    
    status_t HttpConnection::processResponseBody()
    {
        bodyContentsAvailable_=true;
        if (!reader_.get())
            reader_.reset(chunkedEncoding_?new ChunkedBodyReader(*this):new BodyReader(*this));
        return processBodyContents(*reader_);
    }

    HttpConnection::BodyReader::BodyReader(HttpConnection& conn):
        connection_(conn),
        charsRead_(0)
    {
    }  
    
    status_t HttpConnection::BodyReader::readNextChunk()
    {
        connection_.bodyContentsAvailable_=false;
        const uint_t sizeBefore=connection_.response().length();
        status_t error=connection_.SimpleSocketConnection::notifyReadable();
        connection_.finished_=(sizeBefore==connection_.response().length());
        if (eof())
            log().debug()<<_T("BodyReader::readNextChunk(): last read didn't change data length, setting eof flag");
        return error;
    }
    
    void HttpConnection::BodyReader::flush()
    {
        eraseStart(body(), charsRead_);
        charsRead_=0;
    }
    
    status_t HttpConnection::BodyReader::read(int& chr) 
    {
        if (body().length()==charsRead_) 
        {
            if (eof())
            {
                log().debug()<<_T("BodyReader::read(): server shut socket down and buffer is empty, returning npos");
                chr=npos;
                flush();
                return errNone;
            }
            status_t error=readNextChunk();
            if (errNone!=error)
                return error;
            if (body().length()==charsRead_)
            {
                log().debug()<<_T("BodyReader::read(): body().length() didn't change, returning npos");
                chr=npos;
                flush();
                return errNone;
            }
            else
                goto charAvailable;
        }
        else 
        {
charAvailable:        
            ++connection_.readContentLength_;
            chr=body()[charsRead_++];
        }            
        if (chunkLength==charsRead_) 
            flush();
        return errNone;
    }
    
    status_t HttpConnection::BodyReader::read(int& num, String& dst, int offset, int range) 
    {
        if (body().empty() && eof())
        {
            log().debug()<<_T("BodyReader::read(): body().empty() and eof(), returning npos");
            num=npos;
            return errNone;
        }
        for (int pos=0; pos<range; pos++)
        {
            int chr;
            status_t error=read(chr);
            if (errNone!=error)
                return error;
            if (npos!=chr)
                dst[offset+pos]=chr;
            else
            {
                num=pos;
                log().debug()<<_T("BodyReader::read(): encountered npos, last part of data: ")<<String(dst, offset, num);
                return errNone;
            }
        }
        num=range;
        return errNone;      
    }

    status_t HttpConnection::processBodyContents(Reader& reader) 
    {
        while (bodyContentsAvailable()) 
        {
            int chr;
            status_t error=reader.read(chr);
            if (errNone!=error)
                return error;
            if (reader.npos==chr)
                break;
            char_t c=chr;                
        }                   
        return errNone;
    }

    status_t HttpConnection::notifyFinished()
    {
        if (!insideResponseBody_)
            return processResponseHeaders(true);
        return errNone;
    }
    
    status_t HttpConnection::notifyProgress()
    {
        if (!insideResponseBody_)
            return processResponseHeaders(false);
        return errNone;
    }

    HttpConnection::ChunkedBodyReader::ChunkedBodyReader(HttpConnection& conn):
        BodyReader(conn),
        state_(stateInHeader),
        chunkPosition_(0),
        chunkLength_(0)
    {}

    status_t HttpConnection::ChunkedBodyReader::read(int& chr) 
    {
        status_t error;
        int c;
    start:
        if (stateAfterBodyCr==state_ || stateAfterLastChunkCr==state_)
        {
            if (stateAfterBodyCr==state_)
                log().debug()<<_T("ChunkedBodyReader::read(): stateAfterBodyCr");
            else 
                log().debug()<<_T("ChunkedBodyReader::read(): stateAfterLastChunkCr");
            error=BodyReader::read(c);
            if (errNone!=error)
                return error;
            if (_T('\r')!=c)
                return SocketConnection::errResponseMalformed;
            state_=stateAfterBodyCr==state_?stateAfterBodyLf:stateAfterLastChunkLf;
        }
        if (stateAfterBodyLf==state_ || stateAfterLastChunkLf==state_)
        {
            if (stateAfterBodyLf==state_)
                log().debug()<<_T("ChunkedBodyReader::read(): stateAfterBodyLf");
            else 
                log().debug()<<_T("ChunkedBodyReader::read(): stateAfterLastChunkLf");
            error=BodyReader::read(c);
            if (errNone!=error)
                return error;
            if (_T('\n')!=c)
                return SocketConnection::errResponseMalformed;
            if (stateAfterBodyLf==state_)
            {
                state_=stateInHeader;
                chunkHeader_.clear();
            }
            else
            {
                state_=stateFinished;
                flush();
                connection_.insideResponseBody_=false;
                connection_.chunkedBodyFinished_=true;
                connection_.insideResponseHeaders_=true;
                chr=npos;
                connection_.registerEvent(SocketSelector::eventException);
                connection_.registerEvent(SocketSelector::eventRead);
                return errNone;
            }
        }
        if (stateInHeader==state_)
        {
            log().debug()<<_T("ChunkedBodyReader::read(): stateInHeader");
            while (stateInHeader==state_)
            {
                error=BodyReader::read(c);
                if (errNone!=error)
                    return error;
                if (npos==c)
                    return SocketConnection::errResponseMalformed;
                if (_T('\r')==c)
                {
                    state_=stateAfterHeader;
                    error=parseChunkHeader();
                    if (errNone!=error)
                        return error;
                    log().debug()<<_T("ChunkedBodyReader::read(): chunkLength_: ")<<chunkLength_;
                    chunkPosition_=0;
                }
                else
                    chunkHeader_.append(1, c);
            }
        }
        if (stateAfterHeader==state_)
        {
            log().debug()<<_T("ChunkedBodyReader::read(): stateAfterHeader");
            error=BodyReader::read(c);
            if (errNone!=error)
                return error;
            if (_T('\n')!=c)
                return SocketConnection::errResponseMalformed;
            if (0!=chunkLength_)                
                state_=stateInBody;
            else
            {
                state_=stateAfterLastChunkCr;
                goto start;
            }
        }
        if (stateFinished==state_)
        {
            log().debug()<<_T("ChunkedBodyReader::read(): stateFinished");
            chr=npos;
            return errNone;
        }
        assert(stateInBody==state_);
        assert(chunkPosition_<chunkLength_);
        error=BodyReader::read(chr);
        if (errNone!=error)
            return error;
        if (++chunkPosition_==chunkLength_)
            state_=stateAfterBodyCr;
        else if (npos==chr)
            return errHttpUnexpectedEndOfChunk;
        return errNone;
    }   
    
    status_t HttpConnection::ChunkedBodyReader::parseChunkHeader()
    {
        String::size_type end=chunkHeader_.find_first_of(_T("; \t"));
        if (chunkHeader_.npos==end)
            end=chunkHeader_.length();
        long val;
        status_t error=numericValue(chunkHeader_.data(), chunkHeader_.data()+end, val, 16);
        if (errNone!=error)
            return SocketConnection::errResponseMalformed;
        chunkLength_=val;
        return errNone;
    }   
    
    HttpConnection::ChunkedBodyReader::~ChunkedBodyReader()
    {}
    
    HttpConnection::BodyReader::~BodyReader()
    {}

}