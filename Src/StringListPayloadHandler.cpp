#include <StringListPayloadHandler.hpp>

StringListPayloadHandler::~StringListPayloadHandler()
{}

status_t StringListPayloadHandler::handleIncrement(const char_t* payload, ulong_t& length, bool finish)
{
    status_t error=LineBufferedPayloadHandler::handleIncrement(payload, length, finish);
    if (finish && errNone == error)
        error = notifyFinished();
    return error;
}

status_t StringListPayloadHandler::handleLine(const char_t* line, ulong_t length)
{
    volatile status_t error=errNone;
    ErrTry {
        strings.push_back(String(line, length));
    }
    ErrCatch (ex) {
        error = ex;
    } ErrEndCatch
    return error;
}

StringListPayloadHandler::StringListPayloadHandler()
{}

status_t StringListPayloadHandler::notifyFinished()
{
    return errNone;
}
