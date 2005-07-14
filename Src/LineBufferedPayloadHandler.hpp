#ifndef ARSLEXIS_LINE_BUFFERED_PAYLOAD_HANDLER_HPP__
#define ARSLEXIS_LINE_BUFFERED_PAYLOAD_HANDLER_HPP__

#include <IncrementalProcessor.hpp>

class LineBufferedPayloadHandler: public TextIncrementalProcessor {
    
    char_t* buffer_;
	ulong_t bufferLen_;
    char_t delimiter_;
    
protected:
    
    virtual status_t handleLine(const char_t* line, ulong_t len) = 0;
    
    void setDelimiter(char_t delimiter)
    {delimiter_ = delimiter;}

public:

    LineBufferedPayloadHandler();

    ~LineBufferedPayloadHandler();
    
    status_t handleIncrement(const char_t* payload, ulong_t& length, bool finish);
};

#endif