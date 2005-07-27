#ifndef ARSLEXIS_STRING_LIST_PAYLOAD_HANDLER_HPP__
#define ARSLEXIS_STRING_LIST_PAYLOAD_HANDLER_HPP__

#include <LineBufferedPayloadHandler.hpp>
#include <vector>

class StringListPayloadHandler: public LineBufferedTextProcessor 
{
public:

    StringListPayloadHandler();

    ~StringListPayloadHandler();
    
	char_t** strings;
	ulong_t stringsCount;
    
    virtual status_t notifyFinished();

    status_t handleIncrement(const char_t * payload, ulong_t& length, bool finish);
    
protected:
    
    status_t handleLine(const char_t* line, ulong_t length);
    
};

#endif