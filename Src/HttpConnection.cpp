#include <HttpConnection.hpp>
#include <Text.hpp>

#ifdef __MWERKS__
#pragma pcrelconstdata on
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
            "OPTIONS",
            "GET",
            "HEAD",
            "POST",
            "PUT",
            "DELETE",
            "TRACE",
            "CONNECT"
        };
        
        const char_t* crLf="\r\n";

    }

    HttpConnection::HttpConnection(SocketConnectionManager& manager):
        SimpleSocketConnection(manager),
        protocolVersionMajor_(1),
        protocolVersionMinor_(1),
        requestMethod_(methodGet),
        uri_("/"),
        insideResponseHeaders_(false),
        insideResponseBody_(false),
        chunkedEncoding_(false),
        skippingInfoResponse_(false),
        bodyContentsAvailable_(false),
        finished_(false),
        chunkedBodyFinished_(false),
        contentLength_(contentLengthUnavailable),
        readContentLength_(0)
    {}        

    HttpConnection::~HttpConnection() 
    {}

    void HttpConnection::renderRequestLine(String& out) 
    {
        assert(requestMethod_<requestMethodsCount);
        const char_t* method=requestMethods[requestMethod_];
        static const int versionBufferLength=16;
        char_t versionBuffer[versionBufferLength];
        uint_t major=protocolVersionMajor_;
        uint_t minor=protocolVersionMinor_;
        int verLen=StrPrintF(versionBuffer, "%hu.%hu", major, minor);
        out.append(method).append(1, ' ').append(uri_).append(" HTTP/", 6).append(versionBuffer, verLen).append(crLf);
    }

    void HttpConnection::renderHeaderField(String& out, const RequestField_t& field)
    {
        out.append(field.first).append(": ", 2).append(field.second).append(crLf);
    }

    void HttpConnection::commitRequest() 
    {
        String request;
        renderRequestLine(request);
        RequestFields_t::const_iterator end=requestFields_.end();
        for (RequestFields_t::const_iterator it=requestFields_.begin(); it!=end; ++it)
            renderHeaderField(request, *it);
        requestFields_.clear();
        request.append(crLf);
        if (!messageBody_.empty())
        {
            request.append(messageBody_);
            messageBody_.clear();
        }
        setRequest(request);
    }

    Err HttpConnection::handleResponseField(const String& field, const String& value)
    {
        Err error=errNone;
        if (equalsIgnoreCase(field, "Transfer-Encoding"))
        {
            if (equalsIgnoreCase(value, "chunked"))
                chunkedEncoding_=true;
            else 
                error=errHttpUnknownTransferEncoding;
        }
        return error;
    }

    Err HttpConnection::handleStatusLine(uint_t versionMajor, uint_t versionMinor, uint_t statusCode, const String& reason)
    {
        Err error=errNone;
        if (statusCode>=100 && statusCode<200)
            skippingInfoResponse_=true;
        else if (statusCode!=200)
            error=errHttpUnsupportedStatusCode;
        return error;
    }
    
    Err HttpConnection::open() 
    {
        commitRequest();
        return SimpleSocketConnection::open();
    }

    void HttpConnection::addRequestHeader(const String& field, const String& value) 
    {
        requestFields_.push_back(std::make_pair(field, value));
    }
    
    void HttpConnection::setUri(const String& uri) 
    {
        uri_=uri;
        static const int prefixLength=7;
        if (uri.find("http://")==0)
        {
            String::size_type end=uri.find("/", prefixLength);
            String address(uri, prefixLength, end-prefixLength);
            if (address.npos==address.find(':'))
                address.append(":80", 3);
            setAddress(address);            
        }
    }
    
    Err HttpConnection::notifyReadable() 
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
        resp.erase(0, resp.npos==pos?pos:pos+2);
        return true;
    }
    
    Err HttpConnection::processResponseHeaders(bool finish)
    {
        Err error=errNone;
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
    
    Err HttpConnection::processStatusLine(const String& line)
    {
        if (line.find("HTTP/")!=0)
            return errResponseMalformed;
        String::size_type pos0=line.find('.', 6);
        if (line.npos==pos0)
            return errResponseMalformed;
        long value;
        Err error=numericValue(&line[5], &line[pos0], value);
        if (error)
            return errResponseMalformed;
        uint_t major=value;
        String::size_type pos1=line.find_first_of(" \t", pos0+1);
        if (line.npos==pos1)
            return errResponseMalformed;
        error=numericValue(&line[pos0+1],&line[pos1], value);
        if (error)
            return errResponseMalformed;
        uint_t minor=value;
        pos0=line.find_first_not_of(" \t", pos1);
        if (line.npos==pos0)
            return errResponseMalformed;
        pos1=line.find_first_of(" \t", pos0);
        if (line.npos==pos1)
            return errResponseMalformed;
        error=numericValue(&line[pos0], &line[pos1], value);
        if (error)
            return errResponseMalformed;
        uint_t statusCode=value;
        pos0=line.find_first_not_of(" \t", pos1);
        if (line.npos==pos0)
            return errResponseMalformed;
        String reason(line, pos0);
        return handleStatusLine(major, minor, statusCode, reason);
    }

    Err HttpConnection::processHeaderLine(const String& line)
    {
        String::size_type pos=line.find(':');
        if (line.npos==pos)
            return errResponseMalformed;
        String field(line, 0, pos);
        String value;
        pos=line.find_first_not_of(" \t", pos+1);
        if (line.npos!=pos)
            value.assign(line, pos, line.npos);
        return handleResponseField(field, value);
    }
    
    Err HttpConnection::processResponseBody()
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
        return connection_.SimpleSocketConnection::notifyReadable();
    }
    
    void HttpConnection::BodyReader::flush()
    {
        body().erase(0, charsRead_);
        charsRead_=0;
    }
    
    status_t HttpConnection::BodyReader::read(int& chr) 
    {
        if (body().length()==charsRead_) 
        {
            if (eof())
            {
                chr=npos;
                flush();
                return errNone;
            }
            status_t error=readNextChunk();
            if (errNone!=error)
                return error;
            if (body().length()==charsRead_)
            {
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
                return errNone;
            }
        }
        return errNone;      
    }

    Err HttpConnection::processBodyContents(Reader& reader) 
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

    Err HttpConnection::notifyFinished()
    {
        if (!insideResponseBody_)
            return processResponseHeaders(true);
        return errNone;
    }
    
    Err HttpConnection::notifyProgress()
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
            error=BodyReader::read(c);
            if (errNone!=error)
                return error;
            if ('\r'!=c)
                return SocketConnection::errResponseMalformed;
            state_=stateAfterBodyCr==state_?stateAfterBodyLf:stateAfterLastChunkLf;
        }
        if (stateAfterBodyLf==state_ || stateAfterLastChunkLf==state_)
        {
            error=BodyReader::read(c);
            if (errNone!=error)
                return error;
            if ('\n'!=c)
                return SocketConnection::errResponseMalformed;
            if (stateAfterBodyLf==state_)
            {
                state_=stateInHeader;
                chunkHeader_.clear();
            }
            else
            {
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
            while (stateInHeader==state_)
            {
                error=BodyReader::read(c);
                if (errNone!=error)
                    return error;
                if (npos==c)
                    return SocketConnection::errResponseMalformed;
                if ('\r'==c)
                {
                    state_=stateAfterHeader;
                    error=parseChunkHeader();
                    if (errNone!=error)
                        return error;
                    chunkPosition_=0;
                }
                else
                    chunkHeader_.append(1, c);
            }
        }
        if (stateAfterHeader==state_)
        {
            error=BodyReader::read(c);
            if (errNone!=error)
                return error;
            if ('\n'!=c)
                return SocketConnection::errResponseMalformed;
            if (0!=chunkLength_)                
                state_=stateInBody;
            else
            {
                state_=stateAfterLastChunkCr;
                goto start;
            }
        }
        assert(stateInBody==state_);
        assert(chunkPosition_<chunkLength_);
        error=BodyReader::read(chr);
        if (errNone!=error)
            return error;
        if (++chunkPosition_==chunkLength_)
            state_=stateAfterBodyCr;
        return errNone;
    }   
    
    status_t HttpConnection::ChunkedBodyReader::parseChunkHeader()
    {
        String::size_type end=chunkHeader_.find_first_of("; \t");
        if (chunkHeader_.npos==end)
            end=chunkHeader_.length();
        long val;
        status_t error=numericValue(chunkHeader_.data(), chunkHeader_.data()+end, val, 16);
        if (errNone!=error)
            return SocketConnection::errResponseMalformed;
        chunkLength_=val;
        return errNone;
    }   
    
}