#include "iPediaConnection.hpp"
#include "iPediaApplication.hpp"
#include "DefinitionParser.hpp"
#include "SysUtils.hpp"
#include "MainForm.hpp"
#include "DeviceInfo.hpp"

using namespace ArsLexis;


iPediaConnection::iPediaConnection(SocketConnectionManager& manager):
    SimpleSocketConnection(manager),
    parser_(0),
    transactionId_(random((UInt32)-1)),
    inPayload_(false),
    payloadIsError_(false),
    definitionNotFound_(false),
    registering_(false),
    formatVersion_(0),
    payloadStart_(0),
    payloadLength_(0),
    processedSoFar_(0),
    historyChange_(historyReplaceForward)
{
}

iPediaConnection::~iPediaConnection()
{
    delete parser_;
}

#define fieldSeparator ": "
#define lineSeparator "\n"
static const uint_t fieldSeparatorLength=2;
static const uint_t lineSeparatorLength=1;

static void appendField(String& out, const char* fieldName, const String& value)
{
    assert(fieldName!=0);
    uint_t fieldLen=StrLen(fieldName);
    uint_t len=out.length()+fieldLen+fieldSeparatorLength+value.length()+lineSeparatorLength;
    out.reserve(len);
    out.append(fieldName, fieldLen).append(fieldSeparator, fieldSeparatorLength).append(value).append(lineSeparator, lineSeparatorLength);
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
#define definitionNotFoundField "Definition-Not-Found"

void iPediaConnection::prepareRequest()
{
    iPediaApplication& app=static_cast<iPediaApplication&>(iPediaApplication::instance());
    String request;
    appendField(request, protocolVersionField, protocolVersion);
    appendField(request, clientVersionField, appVersion);
    char buffer[9];
    StrPrintF(buffer, "%lx", transactionId_);
    appendField(request, transactionIdField, buffer);
    if (chrNull==app.preferences().cookie[0])
        appendField(request, getCookieField, deviceInfoToken());
    else
        appendField(request, cookieField, app.preferences().cookie);
        
    if (!term_.empty())
        appendField(request, getDefinitionField, term_);
        
    registering_=!(app.preferences().serialNumberRegistered || chrNull==app.preferences().serialNumber[0]);
    if (registering_)
        appendField(request, registerField, app.preferences().serialNumber);
        
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
    const char* line=response().data()+start;
    if (start==response().find(definitionNotFoundField, start))
        definitionNotFound_=true;
    else if (start==response().find(formatVersionField, start))
    {
        error=extractFieldIntValue(response(), start, end, value);
        if (!error)
            formatVersion_=value;
    }
    else if (start==response().find(definitionForField, start))
        definitionForTerm_=extractFieldValue(response(), start, end);
    else if ((start==response().find(errorField, start)) || (start==response().find(definitionField, start))) // Assignment here is intentional.
    {
        payloadIsError_=(start==response().find(errorField, start));
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
        String cookie=extractFieldValue(response(), start, end);
        iPediaApplication& app=static_cast<iPediaApplication&>(iPediaApplication::instance());
        if (cookie.length()!=iPediaApplication::Preferences::cookieLength) ;
        // @todo Notify malformed response here.
        else
            StrCopy(app.preferences().cookie, cookie.c_str());
    }

    if (error)
        handleError(error);
}

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
    iPediaApplication& app=static_cast<iPediaApplication&>(Application::instance());
    if (parser_!=0)
    {
        MainForm* form=static_cast<MainForm*>(app.getOpenForm(mainForm));
        if (form)
        {
            parser_->updateDefinition(form->definition());
            switch (historyChange_)
            {
                case historyMoveForward:
                    form->history().moveForward();
                    break;
                
                case historyMoveBack:
                    form->history().moveBack();
                    break;
                
                case historyReplaceForward:
                    form->history().replaceForward(definitionForTerm_);
                    break;
                
                default:
                    assert(false);
            }                    
            form->setDisplayMode(MainForm::showDefinition);
            form->synchronizeWithHistory();
            form->update();
        }
    }
    
    if (registering_ && !payloadIsError_)
        app.preferences().serialNumberRegistered=true;
    
    if (definitionNotFound_)
        FrmAlert(definitionNotFoundAlert);
        
    SimpleSocketConnection::finalize();
}

void iPediaConnection::handleError(Err error)
{
    switch (error)
    {
        case netErrBufTooSmall:
            FrmAlert(definitionTooBigAlert);
            break;
            //! @todo Display meaningful messages in case of other errors as well.
    }
    SimpleSocketConnection::handleError(error);
}
