#include "iPediaConnection.hpp"
#include "iPediaApplication.hpp"
#include "DefinitionParser.hpp"
#include "SysUtils.hpp"
#include "MainForm.hpp"

using namespace ArsLexis;


iPediaConnection::iPediaConnection(SocketConnectionManager& manager):
    SimpleSocketConnection(manager),
    parser_(0),
    transactionId_(random((UInt32)-1)),
    getCookie_(false),
    inPayload_(false),
    payloadIsError_(false),
    formatVersion_(0),
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
#define lineSeparator "\n"
static const unsigned int fieldSeparatorLength=2;
static const unsigned int lineSeparatorLength=1;

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

static String extractFieldValue(const String& text, UInt16 fieldStart, UInt16 fieldEnd)
{
    String value;
    String::size_type pos=text.find(fieldSeparator, fieldStart);
    if (text.npos!=pos && pos+fieldSeparatorLength<fieldEnd)
        value.assign(text, pos+fieldSeparatorLength, fieldEnd-pos-fieldSeparatorLength);
    return value;
}

static Err extractFieldIntValue(const String& text, UInt16 fieldStart, UInt16 fieldEnd, Int32& value, UInt16 base=10)
{
    Err error=errNone;
    String textValue=extractFieldValue(text, fieldStart, fieldEnd);
    if (!textValue.empty())
        error=numericValue(textValue.data(), textValue.data()+textValue.length(), value, base);
    else
        error=sysErrParamErr;
    return error;
}

void iPediaConnection::open()
{
    prepareRequest();
    SimpleSocketConnection::open();
}

void iPediaConnection::processLine(UInt16 start, UInt16 end)
{
    Int32 value=0;
    Err error=errNone;
    if (start==response().find(formatVersionField, start))
    {
        error=extractFieldIntValue(response(), start, end, value);
        if (!error)
            formatVersion_=value;
    }
    else if (start==response().find(definitionForField, start))
        definitionForTerm_=extractFieldValue(response(), start, end);
    else if ((start==response().find(definitionField, start)) ||
        (payloadIsError_=(start==response().find(errorField, start)))) // Assignment here is intentional.
    {
        error=extractFieldIntValue(response(), start, end, value);
        if (!error)
        {
            inPayload_=true;
            payloadStart_=end+lineSeparatorLength;
            payloadLength_=value;
            delete parser_;
            parser_=new DefinitionParser(response(), payloadStart_);
        }
    }
    else if (start==response().find(cookieField, start))
    {
        //! @todo Set cookie in preferences according to field value
        String cookie=extractFieldValue(response(), start, end);
    }
    if (error)
        handleError(error);
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
                    parser_->parseIncrement(payloadStart_+payloadLength_, true);
                inPayload_=false;
                processedSoFar_=payloadStart_+payloadLength_+lineSeparatorLength;
                goOn=true;
            }
            else
            {
                bool finishPayload=(response().length()-payloadStart_>=payloadLength_);
                parser_->parseIncrement(response().length(), finishPayload);
                processedSoFar_=response().length();
                goOn=false;
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
    if (parser_!=0)
    {
        MainForm* form=static_cast<MainForm*>(Application::instance().getOpenForm(mainForm));
        if (form)
        {
            parser_->updateDefinition(form->definition());
            form->setTerm(definitionForTerm_);
            form->setDisplayMode(MainForm::showDefinition);
            form->update();
        }
    }
    SimpleSocketConnection::finalize();
}

void iPediaConnection::handleError(Err error)
{
    SimpleSocketConnection::handleError(error);
}
