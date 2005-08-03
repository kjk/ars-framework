#include <StringListPayloadHandler.hpp>
#include <Text.hpp>

StringListPayloadHandler::~StringListPayloadHandler()
{
	StrArrFree(strings, stringsCount);
}

status_t StringListPayloadHandler::handleIncrement(const char_t* payload, ulong_t& length, bool finish)
{
    status_t error = LineBufferedPayloadHandler::handleIncrement(payload, length, finish);
    if (finish && errNone == error)
        error = notifyFinished();
    return error;
}

status_t StringListPayloadHandler::handleLine(const char_t* line, ulong_t length)
{
	if (NULL == StrArrAppendStrCopy(strings, stringsCount, line, length))
		return memErrNotEnoughSpace;

    return errNone;
}

StringListPayloadHandler::StringListPayloadHandler():
	strings(NULL),
	stringsCount(0)
{}

status_t StringListPayloadHandler::notifyFinished()
{
    return errNone;
}
