#ifndef __ARSLEXIS_HTTP_CONNECTION_HPP__
#define __ARSLEXIS_HTTP_CONNECTION_HPP__

#include <SimpleSocketConnection.hpp>
#include <Reader.hpp>
#include <vector>
#include <utility>
#include <list>
#include <memory>

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
        
        bool insideResponseHeaders_:1;
        bool insideResponseBody_:1;
        bool chunkedEncoding_:1;
        bool skippingInfoResponse_:1;
        bool bodyContentsAvailable_:1;
        bool finished_:1;
        bool chunkedBodyFinished_:1;

    protected:        
        
        static const ulong_t contentLengthUnavailable=(ulong_t)-1;
        
    private:        
        
        ulong_t contentLength_;
        ulong_t readContentLength_;
        
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

        class BodyReader: public Reader {
        protected:
        
            HttpConnection& connection_;
            
        private:
        
            uint_t charsRead_;
            
            //! Number of chars to read before we start removing them from start of body.
            static const uint_t chunkLength=256;
            
            String& body() 
            {return connection_.response();}
            
            status_t readNextChunk();
            
            bool eof() const
            {return connection_.finished_;}
            
        protected:            
            
            void flush();
                
        public:
        
            explicit BodyReader(HttpConnection& conn);
            
            status_t read(int& chr);
            
            status_t read(int& num, String& dst, int offset, int range);
            
        };
        
        class ChunkedBodyReader: public BodyReader {
            
            String chunkHeader_;
            enum State {
                stateInHeader,
                stateAfterHeader,
                stateInBody,
                stateAfterBodyCr,
                stateAfterBodyLf,
                stateAfterLastChunkCr,
                stateAfterLastChunkLf
            } state_;
            ulong_t chunkPosition_;
            ulong_t chunkLength_;
            
            status_t parseChunkHeader();
            
        public:

            explicit ChunkedBodyReader(HttpConnection& conn);

            status_t read(int& chr);
            
        };
        
        friend class BodyReader;
        friend class ChunkedBodyReader;

        typedef std::auto_ptr<BodyReader> ReaderPtr;
        ReaderPtr reader_;
        
        Err processResponseBody();
        
    protected:
    
        Err open();
    
        virtual Err handleResponseField(const String& field, const String& value);
        
        virtual Err handleStatusLine(uint_t versionMajor, uint_t versionMinor, uint_t statusCode, const String& reason);
        
        Err notifyReadable();
        
        Err notifyFinished();
        
        Err notifyProgress();
        
        bool bodyContentsAvailable() const
        {return bodyContentsAvailable_;}
        
        virtual Err processBodyContents(Reader& reader);
        
        ulong_t contentLength() const
        {return contentLength_;}
        
        ulong_t readContentLength() const
        {return readContentLength_;}
        
    };

}

#endif