#include <FieldPayloadProtocolConnection.hpp>
#include <Text.hpp>
#include <Reader.hpp>
#include <DynStr.hpp>

#define lineSeparator _T("\n")
#define lineSeparatorChar _T('\n')
#define lineSeparatorLength 1

namespace ArsLexis
{
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

    // The rules are:
    // - if field has a value, we do "$field" ":" " " "$value" lineSeparator
    // - if field doesn't have value, we do "$field" ":" lineSeparator
    void FieldPayloadProtocolConnection::appendField(String& out, const char_t* name, uint_t nameLength, const char_t* value, uint_t valueLength)
    {
        assert(nameLength>0);
        assert(':' != name[nameLength-1]);

        uint_t len;
        if (valueLength>0)
            len = out.length() + 2 + valueLength + lineSeparatorLength; // 2 is for ":" and " "
        else
            len = out.length() + 1 + lineSeparatorLength; // 1 is for ":"

        out.reserve(len);
        out.append(name, nameLength);
        if (valueLength>0)
        {
            out.append(_T(": "),2);
            out.append(value, valueLength);
        }
        else
            out.append(_T(":"),1);
        out.append(lineSeparator, lineSeparatorLength);
    }

    void FieldPayloadProtocolConnection::startPayload(PayloadHandler* payloadHandler, ulong_t length)
    {
        payloadHandler_.reset(payloadHandler);
        inPayload_ = true;
        payloadLengthLeft_ = length;
        payloadLength_ = length;
    }

    status_t FieldPayloadProtocolConnection::notifyProgress()
    {
        status_t error=errNone;
        if (!(sending() || response().empty()))
            error=processResponseIncrement();
        return error;
    }

    status_t FieldPayloadProtocolConnection::handlePayloadIncrement(const char_t * payload, ulong_t& length, bool finish)
    {
        assert(NULL != payloadHandler_.get());
        return payloadHandler_->handleIncrement(payload, length, finish);
    }

    status_t FieldPayloadProtocolConnection::processResponseIncrement(bool finish)
    {
        bool goOn = false;
        status_t error = errNone;
        char_t * resp = ArsLexis::StrToUtf16(response().c_str());
        char_t * toFree = resp;
        if (NULL == resp)
            return 1; // TODO: better error
        // TODO: on Palm avoid TextToByteStream() - it's just a copy of data (I think)
        do 
        {
            if (!inPayload_)
            {
                long toConsume = ::StrFind(resp, -1, lineSeparatorChar);
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
                    }
                    else
                    {
                        toConsume = tstrlen(resp);
                        error = processLine(toConsume);
                        resp += toConsume;
                        assert(_T('\0') == resp[0]);
                    }
                    char * newResp = Utf16ToStr(resp);
                    if (NULL == newResp)
                    {
                        error = 1;  // TODO: better error
                        goto Exit;
                    }

                    NarrowString newRespString(newResp);
                    setResponse(newRespString);
                    free(newResp);
                }
            }
            else
            {
                ulong_t respLen = tstrlen(resp);
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
                    char * newResp = Utf16ToStr(resp);
                    if (NULL == newResp)
                    {
                        error = 1;  // TODO: better error
                        goto Exit;
                    }

                    NarrowString newRespString(newResp);
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
                    char * newResp = Utf16ToStr(resp);
                    if (NULL == newResp)
                    {
                        error = 1;  // TODO: better error
                        goto Exit;
                    }

                    NarrowString newRespString(newResp);
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
        String resp;
        ByteStreamToText(response(), resp); // TODO: replace with TxtToUtf16()
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
            return handleField(name, value);

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
        return handleField(name, value);
    }
    
    status_t FieldPayloadProtocolConnection::notifyFinished()
    {
        status_t error=SimpleSocketConnection::notifyFinished();
        if (!error)
            error=processResponseIncrement(true);
        return error;    
    }
    
}

status_t FeedHandlerFromReader(ArsLexis::FieldPayloadProtocolConnection::PayloadHandler& handler, ArsLexis::Reader& reader)
{
    // This function uses String because such is interface of PayloadHandler that it requires so.
    // TODO: refactor PayloadHandler to use const char_t* instead.
    String str;
    
    status_t err;
    const uint_t chunk = 8192;
    uint_t length = chunk;
    while (chunk == length)
    {
        ulong_t before = str.length();
        str.resize(before + length);
            
        if (errNone != (err = reader.read(&str[before], length)))
            return err;
            
        str.resize(before += length);
        
        if (errNone != (err = handler.handleIncrement(str.c_str(), before, chunk != length)))
            return err;
            
        str.erase(0, before);
    }
    return errNone;
}
