#include "iPediaConnection.hpp"
#include "iPediaApplication.hpp"
#include "DefinitionParser.hpp"
#include "SysUtils.hpp"

using namespace ArsLexis;


iPediaConnection::iPediaConnection(SocketConnectionManager& manager):
    SimpleSocketConnection(manager),
    parser_(0),
    transactionId_(random((UInt32)-1)),
    getCookie_(false),
    gotFormatVersion_(false),
    inPayload_(false),
    payloadStart_(0),
    payloadLength_(0),
    processedSoFar_(0)
{
}

iPediaConnection::~iPediaConnection()
{
    delete parser_;
}

#define fieldSeparator ": "
#define lineSeparator "\r\n"

static const unsigned int lineSeparatorLength=2;

static void appendField(String& out, const char* fieldName, const String& value)
{
    assert(fieldName!=0);
    out+=fieldName;
    out+=fieldSeparator;
    out+=value;
    out+=lineSeparator;
}

#define protocolVersion "1"

#define transactionIdField       "Transaction-ID"
#define protocolVersionField    "Protocol-Version"
#define clientVersionField        "Client-Version"
#define getCookieField            "Get-Cookie"
#define getDefinitionField        "Get-Definition"
#define cookieField                 "Cookie"
#define registerField               "Register"
#define formatVersionField      "Format-Version"
#define definitionForField         "Definition-For"
#define definitionField              "Definition"
#define errorField                   "Error"

void iPediaConnection::prepareRequest()
{
    iPediaApplication& app=static_cast<iPediaApplication&>(iPediaApplication::instance());
    String request;
    appendField(request, protocolVersionField, protocolVersion);
    appendField(request, clientVersionField, appVersion);
    char buffer[9];
    StrPrintF(buffer, "%lx", transactionId_);
    appendField(request, transactionIdField, buffer);
    if (getCookie_)
        appendField(request, getCookieField, deviceIdToken());
    else
    {
        //! @todo Obtain cookie stored in application's preferences here and create Cookie field.
    }        
    if (!term_.empty())
        appendField(request, getDefinitionField, term_);
    if (!serialNumber_.empty())
        appendField(request, registerField, serialNumber_);
        
    request+=lineSeparator;
    setRequest(request); 
}

Err iPediaConnection::open(const SocketAddress& address, Int32 timeout)
{
    prepareRequest();
    return SimpleSocketConnection::open(address, timeout);
}

void iPediaConnection::processLine(UInt16 start, UInt16 end)
{
    //! @todo Implement iPediaConnection:processLine()
}

//! @todo Test processResponseIncrement()
void iPediaConnection::processResponseIncrement(bool finish)
{
    bool goOn=false;
    do 
    {
        if (!inPayload_)
        {
            String::size_type end=response().find(lineSeparator, processedSoFar_);
            goOn=(response().npos!=end);
            if (finish || goOn)
            {
                if (!goOn)
                    end=response().length();
                processLine(processedSoFar_, end);
                processedSoFar_=end+lineSeparatorLength;
            }
        }
        else
        {
            assert(parser_!=0);
            if (response().length()-payloadStart_>=payloadLength_+lineSeparatorLength)
            {
                if (processedSoFar_<payloadStart_+payloadLength_)
                    parser_->parseIncrement(payloadLength_, true);
                inPayload_=false;
                processedSoFar_=payloadStart_+payloadLength_+lineSeparatorLength;
                goOn=true;
            }
            else
            {
                bool finishPayload=(response().length()-payloadStart_>=payloadLength_);
                parser_->parseIncrement(response().length()-payloadStart_, finishPayload);
                processedSoFar_=response().length();
            }
        }
    } while (goOn);
}

void iPediaConnection::reportProgress()
{
    if (!(sending() || response().empty()))
        processResponseIncrement();
}

void iPediaConnection::finalize()
{
    processResponseIncrement(true);
    SimpleSocketConnection::finalize();
}