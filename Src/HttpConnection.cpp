#include <HttpConnection.hpp>
#include <Text.hpp>

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
        insideResposeHeaders_(false),
        insideResponseBody_(false),
        chunkedEncoding_(false),
        skippingInfoResponse_(false),
        responseBodyAvailable_(false)
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
            if (equalsIgnoreCase(field, "chunked"))
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
//        if (!insideResponseBody_)
            return SimpleSocketConnection::notifyReadable();
//        else
//            return processResponseBody();
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
                    skippingInfoResponse_=insideResponseBody_=insideResposeHeaders_=false;
                else
                {
                    insideResponseBody_=true;
                    insideResposeHeaders_=false;
                    break;
                }
            }
            else 
            {
                if (!insideResposeHeaders_)
                {
                    error=processStatusLine(line);
                    insideResposeHeaders_=true;
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

}