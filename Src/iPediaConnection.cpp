#include "iPediaConnection.hpp"
#include "iPediaApplication.hpp"
#include "DefinitionParser.hpp"
#include "SysUtils.hpp"
#include "MainForm.hpp"
#include "DeviceInfo.hpp"

using namespace ArsLexis;

iPediaConnection::iPediaConnection(SocketConnectionManager& manager):
    FieldPayloadProtocolConnection(manager),
    transactionId_(random((UInt32)-1)),
    definitionNotFound_(false),
    registering_(false),
    formatVersion_(0),
    definitionParser_(0),
    payloadType_(payloadNone),
    historyChange_(historyReplaceForward),
    serverError_(serverErrorNone)
{
}

iPediaConnection::~iPediaConnection()
{
    delete definitionParser_;
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
        
    request+='\n';
    setRequest(request); 
}

void iPediaConnection::open()
{
    prepareRequest();
    SimpleSocketConnection::open();
}

Err iPediaConnection::handleField(const String& name, const String& value)
{
    std::int32_t numValue;
    Err error=errNone;
    if (0==name.find(transactionIdField))
    {
        //! @todo Handle transactionid field.
    }
    else if (0==name.find(definitionNotFoundField))
        definitionNotFound_=true;
    else if (0==name.find(formatVersionField))
    {
        error=numericValue(value, numValue);
        if (!error)
            formatVersion_=numValue;
        else
            error=errResponseMalformed;
    }
    else if (0==name.find(definitionForField))
        definitionForTerm_=value;
    else if (0==name.find(definitionField))
    {
        error=numericValue(value, numValue);
        if (!error)
        {
            DefinitionParser* parser=new DefinitionParser();
            startPayload(parser, numValue);
            payloadType_=payloadDefinition;
        }
        else
            error=errResponseMalformed;
    }
    else if (0==name.find(cookieField))
    {
        iPediaApplication& app=static_cast<iPediaApplication&>(iPediaApplication::instance());
        if (value.length()!=iPediaApplication::Preferences::cookieLength)
            error=errResponseMalformed;
        else
            app.preferences().cookie=value;
    }
    else if (0==name.find(errorField))
    {
        error=numericValue(value, numValue);
        if (!error)
        {
            if (numValue>=serverErrorFirst && numValue<=serverErrorLast)
                serverError_=static_cast<ServerError>(numValue);
            else
                error=errResponseMalformed;
        }            
        else
            error=errResponseMalformed;
    }
    else 
        error=FieldPayloadProtocolConnection::handleField(name, value);
    return error;
}

Err iPediaConnection::notifyFinished()
{
    Err error=FieldPayloadProtocolConnection::notifyFinished();
    if (!error)
    {
        if (!serverError_)
        {
            iPediaApplication& app=static_cast<iPediaApplication&>(Application::instance());
            if (definitionParser_!=0)
            {
                MainForm* form=static_cast<MainForm*>(app.getOpenForm(mainForm));
                if (form)
                {
                    definitionParser_->updateDefinition(form->definition());
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

            if (registering_ && !serverError_)
                app.preferences().serialNumberRegistered=true;
            
            if (definitionNotFound_)
                app.sendDisplayAlertEvent(definitionNotFoundAlert);
        }
        else
            handleServerError();
    }
    return error;        
}

void iPediaConnection::handleError(Err error)
{
    UInt16 alertId=frmInvalidObjectId;
    switch (error)
    {
        case errResponseTooLong:
            alertId=definitionTooBigAlert;
            break;
            
        case errResponseMalformed:
            alertId=malformedResponseAlert;
            break;
        
        case netErrTimeout:
            alertId=connectionTimedOutAlert;
            break;
        
        default:
            alertId=connectionErrorAlert;
    }
    iPediaApplication& app=static_cast<iPediaApplication&>(Application::instance());
    app.sendDisplayAlertEvent(alertId);
    SimpleSocketConnection::handleError(error);
}

static const UInt16 serverErrorAlerts[]=
{   
    serverFailureAlert,
    unsupportedDeviceAlert,
    invalidAuthorizationAlert,
    malformedRequestAlert
};

void iPediaConnection::handleServerError()
{
    assert(serverErrorNone!=serverError_);
    assert(serverErrorLast>=serverError_);
    FrmAlert(serverErrorAlerts[serverError_-1]);
}

void iPediaConnection::notifyPayloadFinished()
{
    assert(payloadNone!=payloadType_);
    if (payloadDefinition==payloadType_)
    {
        delete definitionParser_;
        definitionParser_=static_cast<DefinitionParser*>(releasePayloadHandler());
    }
    payloadType_=payloadNone;
    FieldPayloadProtocolConnection::notifyPayloadFinished();
}
