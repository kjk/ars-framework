#include <StringListPayloadHandler.hpp>

using ArsLexis::char_t;
using ArsLexis::String;

StringListPayloadHandler::~StringListPayloadHandler()
{}

status_t StringListPayloadHandler::handleIncrement(const char_t* payload, ulong_t& length, bool finish)
{
    status_t error=LineBufferedPayloadHandler::handleIncrement(payload, length, finish);
    if (finish && errNone == error)
        error = notifyFinished();
    return error;
}

status_t StringListPayloadHandler::handleLine(const String& line)
{
    volatile status_t error=errNone;
    ErrTry {
        strings.push_back(line);
    }
    ErrCatch (ex) {
        error=ex;
    } ErrEndCatch
    return error;
}

StringListPayloadHandler::StringListPayloadHandler()
{}

status_t StringListPayloadHandler::notifyFinished()
{
    return errNone;
}
