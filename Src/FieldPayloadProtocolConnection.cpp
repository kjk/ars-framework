#include <FieldPayloadProtocolConnection.hpp>
#include <Text.hpp>

#define lineSeparator _T("\n")
static const uint_t lineSeparatorLength=1;

namespace ArsLexis
{
    FieldPayloadProtocolConnection::PayloadHandler::~PayloadHandler()
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
        inPayload_=true;
        payloadLengthLeft_=payloadLength_=length;
    }

    status_t FieldPayloadProtocolConnection::notifyProgress()
    {
        status_t error=errNone;
        if (!(sending() || response().empty()))
            error=processResponseIncrement();
        return error;
    }

    status_t FieldPayloadProtocolConnection::processResponseIncrement(bool finish)
    {
        bool goOn=false;
        status_t error=errNone;
        String resp;
        ByteStreamToText(response(), resp);
        // TODO: on Palm avoid TextToByteStream() - it's just a copy of data (I think)
        do 
        {
            if (!inPayload())
            {
                String::size_type end=resp.find(lineSeparator);
                goOn=(resp.npos!=end);
                if (finish || goOn)
                {
                    if (!goOn)
                        end=resp.length();
                    error=processLine(end);
                    resp.erase(0, end+1);
                    NarrowString newResp;
                    TextToByteStream(resp, newResp);
                    setResponse(newResp);
                }
            }
            else
            {
                ulong_t length=std::min<ulong_t>(payloadLengthLeft_, resp.length());
                if (resp.length()>=payloadLengthLeft_+lineSeparatorLength)
                {
                    if (length>0)
                        error=payloadHandler_->handleIncrement(resp, length, true);
                    if (!error)
                    {
                        error = notifyPayloadFinished();
                        resp.erase(0, payloadLengthLeft_+lineSeparatorLength);
                        NarrowString newResp;
                        TextToByteStream(resp, newResp);
                        setResponse(newResp);
                        goOn=true;
                    }                        
                }
                else
                {
                    bool finishPayload=(resp.length()>=payloadLengthLeft_);
                    if (errNone==(error=payloadHandler_->handleIncrement(resp, length, finishPayload)))
                    {
                        if (finishPayload)
                            error = notifyPayloadFinished();
                        resp.erase(0, length);
                        NarrowString newResp;
                        TextToByteStream(resp, newResp);
                        setResponse(newResp);
                        payloadLengthLeft_-=length;
                    }                            
                    goOn=false;
                }
            }
        } while (goOn && !error);
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
        ByteStreamToText(response(), resp);
        String::size_type separatorPos=resp.find(_T(":"));

        // empty line means end of response
        if (0==lineEnd)
            return errNone;
        // didn't find the separator => error
        if ((resp.npos==separatorPos) || (separatorPos>=lineEnd))
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
            return errResponseMalformed;

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
