#include <FieldPayloadProtocolConnection.hpp>
#include <Text.hpp>
#include <Reader.hpp>
#include <DynStr.hpp>

#define lineSeparator "\n"
#define lineSeparatorChar '\n'
#define lineSeparatorLength 1

#define fieldSeparatorChar ':'

FieldPayloadProtocolConnection::FieldPayloadProtocolConnection(SocketConnectionManager& manager):
    SimpleSocketConnection(manager),
	payloadHandler_(NULL),
    payloadLengthLeft_(0),
    payloadLength_(0),
    inPayload_(false)
{}

FieldPayloadProtocolConnection::~FieldPayloadProtocolConnection()
{
	delete payloadHandler_;
}

void FieldPayloadProtocolConnection::startPayload(PayloadHandler* payloadHandler, ulong_t length)
{
	delete payloadHandler_;
	payloadHandler_ = payloadHandler;

	inPayload_ = true;
    payloadLengthLeft_ = length;
    payloadLength_ = length;
}

status_t FieldPayloadProtocolConnection::notifyProgress()
{
    status_t error = errNone;
    if (!(sending() || 0 == responseLen_))
        error = processResponseIncrement();
#ifdef DEBUG
    if (errNone != error)
        LogStrUlong(eLogDebug, _T("FieldPayloadProtocolConnection::notifyProgress() returning error: "), error);
#endif
    return error;
}

status_t FieldPayloadProtocolConnection::handlePayloadIncrement(const char* payload, ulong_t& length, bool finish)
{
    assert(NULL != payloadHandler_);
    return payloadHandler_->handleIncrement(payload, length, finish);
}

status_t FieldPayloadProtocolConnection::processResponseIncrement(bool finish)
{
    bool goOn = false;
    status_t error = errNone;

    do 
    {
        if (!inPayload_) 
        {
            long toConsume = StrFind(response_, responseLen_, lineSeparatorChar);
                
            if (-1 == toConsume)
                goOn = false;
            else
                goOn = true;

            if (finish || goOn)
            {
                if (goOn)
                {
                    error = processLine(toConsume);
                    assert(responseLen_ >= ulong_t(toConsume + 1));
                    StrErase(response_, responseLen_, 0, toConsume + 1);
                    responseLen_ -= toConsume + 1;
                }
                else
                {
                    toConsume = responseLen_;
                    error = processLine(toConsume);
                    assert('\0' == response_[toConsume]);
                    free(response_);
                    response_ = NULL;
                    responseLen_ = 0;
                }
            }
        }
        else
        {
            ulong_t length = responseLen_;
            if (length > payloadLengthLeft_)    // this may happen when there are other fields after payload
                length = payloadLengthLeft_;

            if (responseLen_ >= payloadLengthLeft_ + lineSeparatorLength)
            {
                if (length > 0)
                    error = handlePayloadIncrement(response_, length, true);
                if (errNone != error)
                    goto Exit;
                    
                error = notifyPayloadFinished();

                assert(responseLen_ >= payloadLengthLeft_ + lineSeparatorLength);
                StrErase(response_, responseLen_, 0, payloadLengthLeft_ + lineSeparatorLength);
                responseLen_ -= payloadLengthLeft_ + lineSeparatorLength;
                goOn = true;
            }
            else
            {
                bool finishPayload = false;
                if (responseLen_ >= payloadLengthLeft_)
                    finishPayload = true;
                    
                error = handlePayloadIncrement(response_, length, finishPayload);
                if (errNone != error)
                    goto Exit;

                if (finishPayload)
                    error = notifyPayloadFinished();

                assert(responseLen_ >= length);
                StrErase(response_, responseLen_, 0, length);
                responseLen_ -= length;
                payloadLengthLeft_ -= length;
                goOn = false;
            }
        }
    } while (goOn && !error);
    
Exit:
    return error;
}

// The rules are:
// - if field has a value, we do "$field" ":" " " "$value" lineSeparator
// - if field doesn't have value, we do "$field" ":" lineSeparator 
// We extract field and value, passing them to handleField()
status_t FieldPayloadProtocolConnection::processLine(ulong_t lineEnd)
{
    // empty line means end of response
    if (0 == lineEnd)
        return errNone;

    const char* resp = response_;
    long separatorPos = StrFind(resp, lineEnd, fieldSeparatorChar);

    assert(-1 != separatorPos);
    if (-1 == separatorPos)
        return errResponseMalformed;

    assert(ulong_t(separatorPos) < lineEnd);
    if (ulong_t(separatorPos) >= lineEnd)
        return errResponseMalformed;

    const char* name = resp;
    ulong_t nameLen = separatorPos;
    strip(name, nameLen);
    
    // if ":" is at the end, it means a field with no value
    // TODO: should we detect fields with arguments that shouldn't have arguments
    // and vice-versa?
    if (ulong_t(separatorPos + 1) == lineEnd)
        return handleField(name, nameLen, NULL, 0);

    ulong_t valueStartPos = separatorPos + 1;
    assert(valueStartPos <= lineEnd);
    const char* value = resp + valueStartPos;
    ulong_t valueLen = lineEnd - valueStartPos;
    strip(value, valueLen);

    return handleField(name, nameLen, value, valueLen);
}

status_t FieldPayloadProtocolConnection::notifyFinished()
{
    status_t error = SimpleSocketConnection::notifyFinished();
    if (!error)
        error = processResponseIncrement(true);
#ifdef DEBUG
        if (errNone != error)
            LogStrUlong(eLogDebug, _T("FieldPayloadProtocolConnection::notifyFinished() has error"), error);
#endif
    return error;    
}

BinaryIncrementalProcessor* FieldPayloadProtocolConnection::releasePayloadHandler()
{
	BinaryIncrementalProcessor* h = payloadHandler_;
	payloadHandler_ = NULL;
	return h;
}

