#include "FieldPayloadProtocolConnection.hpp"

namespace ArsLexis
{

#define fieldSeparator ": "
#define lineSeparator "\n"
    static const uint_t fieldSeparatorLength=2;
    static const uint_t lineSeparatorLength=1;

    void FieldPayloadProtocolConnection::appendField(String& out, const char* name, uint_t nameLength, const char* value, uint_t valueLength)
    {
        uint_t length=out.length()+nameLength+lineSeparatorLength;
        if (valueLength)
            length+=fieldSeparatorLength+valueLength;
        out.reserve(length);
        out.append(name, nameLength);
        if (valueLength)
            out.append(fieldSeparator, fieldSeparatorLength).append(value, valueLength);
        out.append(lineSeparator, lineSeparatorLength);
    }

    FieldPayloadProtocolConnection::~FieldPayloadProtocolConnection()
    {
        delete payloadHandler_;
    }

    void FieldPayloadProtocolConnection::startPayload(PayloadHandler* payloadHandler, uint_t length)
    {
        delete payloadHandler_;
        payloadStart_=responsePosition_;
        payloadLength_=length;
        payloadHandler_=payloadHandler;
        payloadHandler_->initialize(response(), payloadStart_);
    }

    Err FieldPayloadProtocolConnection::notifyProgress()
    {
        Err error=errNone;
        if (!(sending() || response().empty()))
            error=processResponseIncrement();
        return error;
    }

    Err FieldPayloadProtocolConnection::processResponseIncrement(bool finish)
    {
        bool goOn=false;
        Err error=errNone;
        const String& resp=response();
        do 
        {
            if (0==payloadHandler_)
            {
                String::size_type end=resp.find(lineSeparator, responsePosition_);
                goOn=(resp.npos!=end);
                if (finish || goOn)
                {
                    if (!goOn)
                        end=resp.length();
                    error=processLine(end);
                }
            }
            else
            {
                if (resp.length()-payloadStart_>=payloadLength_+lineSeparatorLength)
                {
                    if (responsePosition_<payloadStart_+payloadLength_)
                    {
                        if (errNone==(error=payloadHandler_->handleIncrement(payloadStart_+payloadLength_, true)))
                        {
                            notifyPayloadFinished();
                            responsePosition_=payloadStart_+payloadLength_+lineSeparatorLength;
                            goOn=true;
                        }
                    }
                }
                else
                {
                    bool finishPayload=(resp.length()-payloadStart_>=payloadLength_);
                    if (errNone==(error=payloadHandler_->handleIncrement(resp.length(), finishPayload)))
                    {
                        if (finishPayload)
                            notifyPayloadFinished();
                        responsePosition_=resp.length();
                    }                            
                    goOn=false;
                }
            }
        } while (goOn && !error);
        return error;
    }
 
    Err FieldPayloadProtocolConnection::processLine(uint_t lineEnd)
    {
        String name, value;
        const String& resp=response();
        String::size_type separatorPos=resp.find(fieldSeparator, responsePosition_);
        if (resp.npos!=separatorPos && separatorPos<lineEnd)
        {
            name.assign(resp, responsePosition_, separatorPos-responsePosition_);
            separatorPos+=fieldSeparatorLength;
            value.assign(resp, separatorPos, lineEnd-separatorPos);
        }
        else
            name.assign(resp, responsePosition_, lineEnd-responsePosition_);
        responsePosition_=lineEnd+lineSeparatorLength;
        return (name.length()?handleField(name, value):errNone);
    }
    
    Err FieldPayloadProtocolConnection::notifyFinished()
    {
        Err error=SimpleSocketConnection::notifyFinished();
        if (!error)
            error=processResponseIncrement(true);
        return error;    
    }
    
    
}