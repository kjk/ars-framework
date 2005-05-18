#include <FieldPayloadProtocolConnection.hpp>
#include <Text.hpp>
#include <Reader.hpp>
#include <DynStr.hpp>

#define lineSeparator _T("\n")
#define lineSeparatorChar _T('\n')
#define lineSeparatorLength 1

FieldPayloadProtocolConnection::PayloadHandler::~PayloadHandler()
{}

FieldPayloadProtocolConnection::FieldPayloadProtocolConnection(SocketConnectionManager& manager):
    SimpleSocketConnection(manager),
    payloadLengthLeft_(0),
    payloadLength_(0),
    inPayload_(false)
{}

FieldPayloadProtocolConnection::~FieldPayloadProtocolConnection()
{}

void FieldPayloadProtocolConnection::startPayload(PayloadHandler* payloadHandler, ulong_t length)
{
    payloadHandler_.reset(payloadHandler);
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
        LogStrUlong(eLogDebug, _T("FieldPayloadProtocolConnection::notifyProgress() has error"), error);
#endif
    return error;
}

status_t FieldPayloadProtocolConnection::handlePayloadIncrement(const char_t* payload, ulong_t& length, bool finish)
{
    assert(NULL != payloadHandler_.get());
    return payloadHandler_->handleIncrement(payload, length, finish);
}

status_t FieldPayloadProtocolConnection::processResponseIncrement(bool finish)
{
    bool goOn = false;
    status_t error = errNone;
    ulong_t respLen = response().size();
    char_t* resp = StrToUtf16(response().data(), respLen);
    char_t* toFree = resp;
    if (NULL == resp)
        return memErrNotEnoughSpace;
    // TODO: on Palm avoid TextToByteStream() - it's just a copy of data (I think)
    do 
    {
        if (!inPayload_)
        {
            long toConsume = StrFind(resp, -1, lineSeparatorChar);
            if (-1 == toConsume)
                goOn = false;
            else
                goOn = true;

            if (finish || goOn)
            {
                if (goOn)
                {
                    error = processLine(toConsume);
                    resp += toConsume+1; // swallow also lineSeparator
                    assert( respLen >= ulong_t(toConsume + 1));
                    respLen -= (toConsume+1);
                }
                else
                {
                    toConsume = tstrlen(resp);
                    error = processLine(toConsume);
                    resp += toConsume;
                    assert(_T('\0') == resp[0]);
                }
                char * newResp = Utf16ToStr(resp, respLen);
                if (NULL == newResp)
                {
                    error = memErrNotEnoughSpace;  

                    goto Exit;
                }

                NarrowString newRespString;
                newRespString.assign(newResp, respLen);
                setResponse(newRespString);
                free(newResp);
            }
        }
        else
        {
            ulong_t length = respLen;
            if (length > payloadLengthLeft_)    // TODO: is it even possible?
                length = payloadLengthLeft_;

            if (respLen >= payloadLengthLeft_ + lineSeparatorLength)
            {
                if (length > 0)
                    error = handlePayloadIncrement(resp, length, true);
                if (errNone!=error)
                    goto Exit;
                error = notifyPayloadFinished();

                resp += payloadLengthLeft_ + lineSeparatorLength;
                assert( respLen >= payloadLengthLeft_ + lineSeparatorLength);
                respLen -= (payloadLengthLeft_ + lineSeparatorLength);
                char * newResp = Utf16ToStr(resp, respLen);
                if (NULL == newResp)
                {
                    error = memErrNotEnoughSpace;
                    goto Exit;
                }

                NarrowString newRespString;
                newRespString.assign(newResp, respLen);
                setResponse(newRespString);
                free(newResp);
                goOn = true;
            }
            else
            {
                bool finishPayload = false;
                if (respLen >= payloadLengthLeft_)
                    finishPayload = true;
                error = handlePayloadIncrement(resp, length, finishPayload);
                if (errNone!=error)
                    goto Exit;

                if (finishPayload)
                    error = notifyPayloadFinished();

                resp += length;
                assert( respLen >= length);
                respLen -= length;
                char * newResp = Utf16ToStr(resp, respLen);
                if (NULL == newResp)
                {
                    error = memErrNotEnoughSpace;
                    goto Exit;
                }

                NarrowString newRespString;
                newRespString.assign(newResp, respLen);
                setResponse(newRespString);
                free(newResp);

                payloadLengthLeft_ -= length;
                goOn = false;
            }
        }
    } while (goOn && !error);
Exit:
    if (NULL != toFree)
        free(toFree);
    return error;
}

// The rules are:
// - if field has a value, we do "$field" ":" " " "$value" lineSeparator
// - if field doesn't have value, we do "$field" ":" lineSeparator 
// We extract field and value, passing them to handleField()
status_t FieldPayloadProtocolConnection::processLine(uint_t lineEnd)
{
    String name, value;
#ifdef _PALM_OS
    String resp = response();
#else
    String resp;
    ByteStreamToText(response(), resp); // TODO: replace with TxtToUtf16()
#endif

    String::size_type separatorPos=resp.find(_T(":"));

    // empty line means end of response
    if (0==lineEnd)
        return errNone;

    assert(String::npos!=separatorPos);
    if (String::npos==separatorPos)
        return errResponseMalformed;

    assert(separatorPos<lineEnd);
    if (separatorPos>=lineEnd)
        return errResponseMalformed;

    name.assign(resp, 0, separatorPos);
    assert(separatorPos>0);
    // if ":" is at the end, it means a field with no value
    // TODO: should we detect fields with arguments that shouldn't have arguments
    // and vice-versa?
    if (separatorPos+1==lineEnd)
        return handleField(name.c_str(), NULL);

    // we must have space after ":"
    String::size_type expectedSpacePos = separatorPos+1;
    if (_T(' ') != resp[expectedSpacePos])
    {
        assert(false);
        return errResponseMalformed;
    }

    String::size_type valueStartPos = separatorPos+2;
    assert(valueStartPos<=lineEnd);

    value.assign(resp, valueStartPos,lineEnd-valueStartPos);
    return handleField(name.c_str(), value.c_str());
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
