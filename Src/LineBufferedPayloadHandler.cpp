#include <LineBufferedPayloadHandler.hpp>

LineBufferedPayloadHandler::LineBufferedPayloadHandler():
    delimiter_(_T('\n'))
{}

LineBufferedPayloadHandler::~LineBufferedPayloadHandler()
{}

status_t LineBufferedPayloadHandler::handleIncrement(const char_t * payload, ulong_t& length, bool finish)
{
    lineBuffer_.append(payload, length);
    String::size_type pos = lineBuffer_.find(delimiter_);
    while (true) 
    {
        if (lineBuffer_.npos==pos && !finish)
            break;
        String line(lineBuffer_, 0, pos);
        status_t error=handleLine(line);
        if (errNone!=error)
            return error;
        if (lineBuffer_.npos==pos)
            break;
        lineBuffer_.erase(0, pos+1);
        pos=lineBuffer_.find(delimiter_);
    }
    return errNone;
}
