#ifndef ARSLEXIS_STRING_LIST_PAYLOAD_HANDLER_HPP__
#define ARSLEXIS_STRING_LIST_PAYLOAD_HANDLER_HPP__

#include <LineBufferedPayloadHandler.hpp>
#include <vector>

namespace ArsLexis {

    class StringListPayloadHandler: public LineBufferedPayloadHandler {
    
    public:
    
        StringListPayloadHandler();
    
        ~StringListPayloadHandler();
        
    protected:
        
        status_t handleLine(const String& line);
        
        typedef std::vector<String> Strings_t;

        virtual status_t listReady(Strings_t& strings)=0;
        
        status_t handleIncrement(const char_t * payload, ulong_t& length, bool finish);
        
    private:
        
        Strings_t strings_;    
        
    };
    
}    

#endif