#ifndef __ARSLEXIS_HTTP_CONNECTION_HPP__
#define __ARSLEXIS_HTTP_CONNECTION_HPP__

#include <SimpleSocketConnection.hpp>
#include <vector>
#include <utility>
#include <list>

namespace ArsLexis {

    class HttpConnection: public SimpleSocketConnection {

    public:
    
        explicit HttpConnection(SocketConnectionManager& manager);
        
        ~HttpConnection();

        void setProtocolVersion(uint_t major, uint_t minor)
        {
            protocolVersionMajor_=major;
            protocolVersionMinor_=minor;
        }
        
        enum RequestMethod {
            methodOptions,
            methodGet,
            methodHead,
            methodPost,
            methodPut,
            methodDelete,
            methodTrace,
            methodConnect
        };

        void setRequestMethod(RequestMethod rm)
        {requestMethod_=rm;}
        
        void setMessageBody(const String& mb)
        {messageBody_=mb;}
        
        void setUri(const String& uri);
        
        enum Error {
            errHttpUnknownTransferEncoding=errFirstAvailable,
            errHttpUnsupportedStatusCode,
            errFirstAvailable
        };
        
        void addRequestHeader(const String& field, const String& value);

    private:
    
        uint_t protocolVersionMajor_:4;
        uint_t protocolVersionMinor_:4;
        RequestMethod requestMethod_:8;
        
        bool insideResposeHeaders_:1;
        bool insideResponseBody_:1;
        bool chunkedEncoding_:1;
        bool skippingInfoResponse_:1;
        bool responseBodyAvailable_:1;
        
        String uri_;
        String messageBody_;
        typedef std::pair<String, String> RequestField_t;
        typedef std::vector<RequestField_t> RequestFields_t;
        RequestFields_t requestFields_;

        void renderRequestLine(String& out);

        void renderHeaderField(String& out, const RequestField_t& field);

        void commitRequest();
        
        typedef std::list<String> Chunks_t;
        Chunks_t chunks_;
        
        bool nextResponseLine(String& line, bool finish);
        
        Err processResponseHeaders(bool finish);
        
        Err processStatusLine(const String& line);
        
        Err processHeaderLine(const String& line);

//        Err processResponseBody();
        
    protected:
    
        Err open();
    
        virtual Err handleResponseField(const String& field, const String& value);
        
        virtual Err handleStatusLine(uint_t versionMajor, uint_t versionMinor, uint_t statusCode, const String& reason);
        
        Err notifyReadable();
        
        Err notifyFinished()
        {return processResponseHeaders(true);}
        
        Err notifyProgress()
        {return processResponseHeaders(false);}
        
    };

}

#endif