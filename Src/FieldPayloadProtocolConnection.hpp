#ifndef __ARSLEXIS_FIELD_PAYLOAD_PROTOCOL_CONNECTION_HPP__
#define __ARSLEXIS_FIELD_PAYLOAD_PROTOCOL_CONNECTION_HPP__

#include <SimpleSocketConnection.hpp>

namespace ArsLexis
{

    class FieldPayloadProtocolConnection: public SimpleSocketConnection
    {
    public:
        
        class PayloadHandler
        {
        public:
        
            virtual ~PayloadHandler()
            {}
            
            virtual Err handleIncrement(const String& payload, ulong_t& length, bool finish)=0;
            
        };
        
    private:
        
        PayloadHandler* payloadHandler_;
        ulong_t payloadLengthLeft_;
        ulong_t payloadLength_;
        
        Err processResponseIncrement(bool finish=false);
        
        Err processLine(uint_t lineEnd);
        
    protected:
    
        virtual Err handleField(const String& name, const String& value)
        {return errResponseMalformed;}
        
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
        
        void startPayload(PayloadHandler* payloadHandler, ulong_t length);
        
        Err notifyProgress();
        
        Err notifyFinished();
        
        bool inPayload() const
        {return payloadHandler_!=0;}
        
        ulong_t payloadLengthLeft() const
        {return payloadLengthLeft_;}
        
        ulong_t payloadLength() const
        {return payloadLength_;}
        
    public:
    
        FieldPayloadProtocolConnection(SocketConnectionManager& manager):
            SimpleSocketConnection(manager),
            payloadHandler_(0),
            payloadLengthLeft_(0),
            payloadLength_(0)
        {}
        
        ~FieldPayloadProtocolConnection();
        
        static void appendField(String& out, const char* name, uint_t nameLength, const char* value=0, uint_t valueLength=0);
        
        static void appendField(String& out, const String& name, const String& value)
        {appendField(out, name.data(), name.length(), value.data(), value.length());}
        
        static void appendField(String& out, const char* name, const char* value=0)
        {appendField(out, name, StrLen(name), value, (value?StrLen(value):0));}
        
    };

}

#endif