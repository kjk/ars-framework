#ifndef __ARSLEXIS_FIELD_PAYLOAD_PROTOCOL_CONNECTION_HPP__
#define __ARSLEXIS_FIELD_PAYLOAD_PROTOCOL_CONNECTION_HPP__

#include <BaseTypes.hpp>
#include <Reader.hpp>
#include <SimpleSocketConnection.hpp>

using ArsLexis::status_t;

#define transactionIdField      _T("Transaction-ID")
#define protocolVersionField    _T("Protocol-Version")
#define clientInfoField         _T("Client-Info")
#define getCookieField          _T("Get-Cookie")
#define cookieField             _T("Cookie")
#define regCodeField            _T("Registration-Code")
#define formatVersionField      _T("Format-Version")
#define errorField              _T("Error")



class FieldPayloadProtocolConnection : public SimpleSocketConnection
{
public:

    class PayloadHandler: private NonCopyable
    {
        public:
            virtual ~PayloadHandler();
            virtual status_t handleIncrement(const char_t * payload, ulong_t& length, bool finish)=0;
    };

private:

    typedef std::auto_ptr<PayloadHandler> PayloadHandlerPtr;
    PayloadHandlerPtr payloadHandler_;
    ulong_t payloadLengthLeft_;
    ulong_t payloadLength_;
    
    status_t processResponseIncrement(bool finish=false);
    
    status_t processLine(uint_t lineEnd);
    
protected:

    virtual status_t handlePayloadIncrement(const char_t* payload, ulong_t& length, bool finish);
    
    virtual status_t handleField(const String& name, const String& value)
    {
        assert(false);
        return errResponseMalformed;
    }
    
    PayloadHandler* releasePayloadHandler()
    {return payloadHandler_.release();}
    
    PayloadHandler* payloadHandler() const
    {return payloadHandler_.get();}
    
    virtual status_t notifyPayloadFinished()
    {
        inPayload_ = false; 
        return errNone;
    }
    
    void startPayload(PayloadHandler* payloadHandler, ulong_t length);
    
    status_t notifyProgress();
    
    status_t notifyFinished();
   
    ulong_t payloadLengthLeft() const
    {return payloadLengthLeft_;}
    
    ulong_t payloadLength() const
    {return payloadLength_;}
    
public:

    FieldPayloadProtocolConnection(SocketConnectionManager& manager);
    
    ~FieldPayloadProtocolConnection();

    bool inPayload_;
};

//class Reader;

status_t FeedHandlerFromReader(FieldPayloadProtocolConnection::PayloadHandler& handler, Reader& reader);

#endif
