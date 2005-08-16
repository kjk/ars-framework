#ifndef __ARSLEXIS_FIELD_PAYLOAD_PROTOCOL_CONNECTION_HPP__
#define __ARSLEXIS_FIELD_PAYLOAD_PROTOCOL_CONNECTION_HPP__

#include <BaseTypes.hpp>
#include <Reader.hpp>
#include <SimpleSocketConnection.hpp>
#include <IncrementalProcessor.hpp>

#define transactionIdField      "Transaction-ID"
#define protocolVersionField    "Protocol-Version"
#define clientInfoField         "Client-Info"
#define getCookieField          "Get-Cookie"
#define cookieField             "Cookie"
#define regCodeField            "Registration-Code"
#define formatVersionField      "Format-Version"
#define errorField              "Error"


class FieldPayloadProtocolConnection : public SimpleSocketConnection
{

    BinaryIncrementalProcessor* payloadHandler_;
    ulong_t payloadLengthLeft_;
    ulong_t payloadLength_;
    
    status_t processResponseIncrement(bool finish = false);
    
    status_t processLine(ulong_t lineEnd);
    
protected:

	typedef BinaryIncrementalProcessor PayloadHandler;

    virtual status_t handlePayloadIncrement(const char* payload, ulong_t& length, bool finish);
    
    virtual status_t handleField(const char* name, ulong_t nameLen, const char* value, ulong_t valueLen)
    {
        assert(false);
        return errResponseMalformed;
    }
    
    PayloadHandler* releasePayloadHandler();
    
    PayloadHandler* payloadHandler() const
    {return payloadHandler_;}
    
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

#endif
