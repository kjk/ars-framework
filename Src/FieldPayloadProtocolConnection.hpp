#ifndef __ARSLEXIS_FIELD_PAYLOAD_PROTOCOL_CONNECTION_HPP__
#define __ARSLEXIS_FIELD_PAYLOAD_PROTOCOL_CONNECTION_HPP__

#include <SimpleSocketConnection.hpp>
#include <BaseTypes.hpp>

#define transactionIdField      _T("Transaction-ID")
#define protocolVersionField    _T("Protocol-Version")
#define clientInfoField         _T("Client-Info")
#define getCookieField          _T("Get-Cookie")
#define cookieField             _T("Cookie")
#define regCodeField            _T("Registration-Code")
#define formatVersionField      _T("Format-Version")
#define errorField              _T("Error")

namespace ArsLexis
{

    class FieldPayloadProtocolConnection: public SimpleSocketConnection
    {
    public:
        
        class PayloadHandler: private ArsLexis::NonCopyable
        {
        public:
        
            virtual ~PayloadHandler();
            
            virtual status_t handleIncrement(const String& payload, ulong_t& length, bool finish)=0;
            
        };
        
    private:
    
        typedef std::auto_ptr<PayloadHandler> PayloadHandlerPtr;
        PayloadHandlerPtr payloadHandler_;
        ulong_t payloadLengthLeft_;
        ulong_t payloadLength_;
        
        status_t processResponseIncrement(bool finish=false);
        
        status_t processLine(uint_t lineEnd);
        
    protected:
    
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
        
        static void appendField(String& out, const char_t* name, uint_t nameLength, const char_t* value=0, uint_t valueLength=0);
        
        static void appendField(String& out, const String& name, const String& value)
        {appendField(out, name.data(), name.length(), value.data(), value.length());}
        
        static void appendField(String& out, const char_t* name, const char_t* value=0)
        {appendField(out, name, tstrlen(name), value, (value?tstrlen(value):0));}
        
        static void appendField(String& out, const char_t* name, const String& value)
        {appendField(out, name, tstrlen(name), value.data(), value.length());}
        
        static void appendField(String& out, const String& name, const char_t* value)
        {appendField(out, name.data(), name.length(), value, (value?tstrlen(value):0));}

        bool inPayload_;

    };

}

#endif
