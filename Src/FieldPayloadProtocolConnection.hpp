#ifndef __ARSLEXIS_FIELD_PAYLOAD_PROTOCOL_CONNECTION_HPP__
#define __ARSLEXIS_FIELD_PAYLOAD_PROTOCOL_CONNECTION_HPP__

#include "SimpleSocketConnection.hpp"

namespace ArsLexis
{

    class FieldPayloadProtocolConnection: public SimpleSocketConnection
    {
    public:
        
        class PayloadHandler
        {
        public:
        
            virtual void initialize(const String& payload, uint_t startOffset)
            {}
            
            virtual ~PayloadHandler()
            {}
            
            virtual Err handleIncrement(uint_t incrementEnd, bool finish)=0;
            
        };
        
    private:
        
        PayloadHandler* payloadHandler_;
        uint_t responsePosition_;
        uint_t payloadStart_;
        uint_t payloadLength_;
        
        Err processResponseIncrement(bool finish=false);
        
        Err processLine(uint_t lineEnd);
        
    protected:
    
        virtual Err handleField(const String& name, const String& value)
        {
            return sockConnErrResponseMalformed;
        }
        
        PayloadHandler* releasePayloadHandler()
        {
            PayloadHandler* handler=payloadHandler_;
            payloadHandler_=0;
            return handler;
        }
        
        virtual void notifyPayloadFinished()
        {
            delete releasePayloadHandler();
        }
        
        void startPayload(PayloadHandler* payloadHandler, uint_t length);
        
        Err notifyProgress();
        
        Err notifyFinished();
        
    public:
    
        FieldPayloadProtocolConnection(SocketConnectionManager& manager):
            SimpleSocketConnection(manager),
            payloadHandler_(0),
            responsePosition_(0),
            payloadStart_(0),
            payloadLength_(0)
        {}
        
        ~FieldPayloadProtocolConnection();
        
        static void appendField(String& out, const char* name, uint_t nameLength, const char* value, uint_t valueLength);
        
        static void appendField(String& out, const String& name, const String& value)
        {appendField(out, name.data(), name.length(), value.data(), value.length());}
        
        static void appendField(String& out, const char* name, const char* value)
        {appendField(out, name, StrLen(name), value, StrLen(value));}
        
    };

}

#endif