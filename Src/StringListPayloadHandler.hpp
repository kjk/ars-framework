#ifndef ARSLEXIS_STRING_LIST_PAYLOAD_HANDLER_HPP__
#define ARSLEXIS_STRING_LIST_PAYLOAD_HANDLER_HPP__

#include <FieldPayloadProtocolConnection.hpp>
#include <vector>

namespace ArsLexis {

    class StringListPayloadHandler: public FieldPayloadProtocolConnection::PayloadHandler {
    
        String buffer_;
        
    public:
    
        ~StringListPayloadHandler();
        
        status_t handleIncrement(const String& payload, ulong_t& length, bool finish);

    protected:
        
        typedef std::vector<String> Strings_t;

        virtual status_t listReady(Strings_t& strings)=0;
        
    };
    
}    

#endif