#include <FieldPayloadProtocolConnection.hpp>

namespace ArsLexis
{


    namespace {

#define fieldSeparator _T(": ")
#define lineSeparator _T("\n")

        static const uint_t fieldSeparatorLength=2;
        static const uint_t lineSeparatorLength=1;
        
    }

    FieldPayloadProtocolConnection::~FieldPayloadProtocolConnection()
    {
        delete payloadHandler_;
    }

    void FieldPayloadProtocolConnection::appendField(String& out, const char_t* name, uint_t nameLength, const char_t* value, uint_t valueLength)
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

    void FieldPayloadProtocolConnection::startPayload(PayloadHandler* payloadHandler, ulong_t length)
    {
        delete payloadHandler_;
        payloadLengthLeft_=payloadLength_=length;
        payloadHandler_=payloadHandler;
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
        String& resp=response();
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
                        notifyPayloadFinished();
                        resp.erase(0, payloadLengthLeft_+lineSeparatorLength);
                        goOn=true;
                    }                        
                }
                else
                {
                    bool finishPayload=(resp.length()>=payloadLengthLeft_);
                    if (errNone==(error=payloadHandler_->handleIncrement(resp, length, finishPayload)))
                    {
                        if (finishPayload)
                            notifyPayloadFinished();
                        resp.erase(0, length);
                        payloadLengthLeft_-=length;
                    }                            
                    goOn=false;
                }
            }
        } while (goOn && !error);
        return error;
    }
 
    status_t FieldPayloadProtocolConnection::processLine(uint_t lineEnd)
    {
        String name, value;
        const String& resp=response();
        String::size_type separatorPos=resp.find(fieldSeparator);
        if (resp.npos!=separatorPos && separatorPos<lineEnd)
        {
            name.assign(resp, 0, separatorPos);
            separatorPos+=fieldSeparatorLength;
            value.assign(resp, separatorPos, lineEnd-separatorPos);
        }
        else
            name.assign(resp, 0, lineEnd);
        return (name.length()?handleField(name, value):errNone);
    }
    
    status_t FieldPayloadProtocolConnection::notifyFinished()
    {
        status_t error=SimpleSocketConnection::notifyFinished();
        if (!error)
            error=processResponseIncrement(true);
        return error;    
    }
    
}
