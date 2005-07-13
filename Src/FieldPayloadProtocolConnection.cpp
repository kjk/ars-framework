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
    if (!(sending() || response().empty()))
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
    ulong_t respLen = response().size();
    const char* resp = response().data();

    do 
    {
        const char* orig = resp;
        if (!inPayload_) 
        {
            long toConsume = StrFind(resp, respLen, lineSeparatorChar);
                
            if (-1 == toConsume)
                goOn = false;
            else
                goOn = true;

            if (finish || goOn)
            {
                if (goOn)
                {
                    error = processLine(toConsume);
                    resp += toConsume + 1; // swallow also lineSeparator
                    assert( respLen >= ulong_t(toConsume + 1));
                    respLen -= (toConsume+1);
                    response().erase(0, toConsume + 1);
                }
                else
                {
                    toConsume = respLen;
                    error = processLine(toConsume);
                    resp += toConsume;
                    assert('\0' == resp[0]);
                    response().clear();
                }
            }
        }
        else
        {
            ulong_t length = respLen;
            if (length > payloadLengthLeft_)    // this may happen when there are other fields after payload
                length = payloadLengthLeft_;

            if (respLen >= payloadLengthLeft_ + lineSeparatorLength)
            {
                if (length > 0)
                    error = handlePayloadIncrement(resp, length, true);
                if (errNone != error)
                    goto Exit;
                    
                error = notifyPayloadFinished();

                resp += payloadLengthLeft_ + lineSeparatorLength;
                assert( respLen >= payloadLengthLeft_ + lineSeparatorLength);
                respLen -= (payloadLengthLeft_ + lineSeparatorLength);
                
                response().erase(0, payloadLengthLeft_ + lineSeparatorLength);         
                goOn = true;
            }
            else
            {
                bool finishPayload = false;
                if (respLen >= payloadLengthLeft_)
                    finishPayload = true;
                    
                error = handlePayloadIncrement(resp, length, finishPayload);
                if (errNone != error)
                    goto Exit;

                if (finishPayload)
                    error = notifyPayloadFinished();

                resp += length;
                assert( respLen >= length);
                respLen -= length;
                
                response().erase(0, length);         
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

    const char* resp = response().data();
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
    if (separatorPos + 1 == lineEnd)
        return handleField(name, nameLen, NULL, 0);

    ulong_t valueStartPos = separatorPos + 2;
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


#ifndef _MSC_VER

// TODO: Redesign to use Reader::readRaw()
status_t FeedHandlerFromReader(FieldPayloadProtocolConnection::PayloadHandler& handler, Reader& reader)
{
    // This function uses String because such is interface of PayloadHandler that it requires so.
    // TODO: refactor PayloadHandler to use const char_t* instead.
    CDynStr str;
    
    status_t err;
    const uint_t chunk = 8192;
    char_t* buffer = (char_t*)malloc(sizeof(char_t) * chunk);
    if (NULL == buffer)
        return memErrNotEnoughSpace;
        
    uint_t length = chunk;
    while (true)
    {
        if (errNone != (err = reader.read(buffer, length)))
            goto Error;

        if (NULL == str.AppendCharPBuf(buffer, length))
        {
            err = memErrNotEnoughSpace;
            goto Error;
        }            
        
        ulong_t incLen = str.Len();
        if (errNone != (err = handler.handleIncrement(str.GetCStr(), incLen, chunk != length)))
            return err;
        
        assert(incLen <= str.Len());
        DynStrRemoveStartLen(&str, 0, incLen);
        if (chunk != length)
            break;
    }
    free(buffer);
    return errNone;
Error:
    if (NULL != buffer)
        free(buffer);
    return err;    
}

#endif