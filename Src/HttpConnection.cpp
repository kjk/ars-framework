#include <HttpConnection.hpp>

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
        protocolVersionMinor_(0)
    {}        

    HttpConnection::~HttpConnection() 
    {
    }

    void HttpConnection::renderRequestLine(String& out) 
    {
        assert(requestMethod_<requestMethodsCount);
        const char_t* method=requestMethods[requestMethod_];
        static const int versionBufferLength=16;
        char_t versionBuffer[versionBufferLength];
        uint_t major=protocolVersionMajor_;
        uint_t minor=protocolVersionMinor_;
        int verLen=StrPrintF(versionBuffer, "%hu.%hu", major, minor);
        out.append(method).append(' ', 1).append(uri_).append(" HTTP/", 6).append(versionBuffer, verLen).append(crLf);
    }

    void HttpConnection::renderHeaderField(String& out, const RequestField_t& field)
    {
        out.append(field.first).append(": ", 2).append(field.second).append(crLf);
    }

    void HttpConnection::renderRequest() 
    {
        String request;
        renderRequestLine(request);
        RequestFields_t::const_iterator end=requestFields_.end();
        for (RequestFields_t::const_iterator it=requestFields_.begin(); it!=end; ++it)
            renderHeaderField(request, *it);
    }

    
}