#ifndef ARSLEXIS_LINE_BUFFERED_PAYLOAD_HANDLER_HPP__
#define ARSLEXIS_LINE_BUFFERED_PAYLOAD_HANDLER_HPP__

#include <FieldPayloadProtocolConnection.hpp>

class LineBufferedPayloadHandler: public FieldPayloadProtocolConnection::PayloadHandler {
    
    String lineBuffer_;
    char_t delimiter_;
    
protected:
    
    virtual status_t handleLine(const String& line)=0;
    
    void setDelimiter(char_t delimiter)
    {delimiter_=delimiter;}

public:

    LineBufferedPayloadHandler();

    ~LineBufferedPayloadHandler();
    
    status_t handleIncrement(const char_t * payload, ulong_t& length, bool finish);
};

#endif