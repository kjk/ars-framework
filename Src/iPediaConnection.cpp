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
    formatVersion_(0),
    definitionParser_(0),
    searchResultsHandler_(0),
    payloadType_(payloadNone),
    historyChange_(historyReplaceForward),
    serverError_(serverErrorNone),
    notFound_(false),
    registering_(false),
    performFullTextSearch_(false)
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
#define resultsForField             "Results-For"
#define definitionField              "Definition"
#define errorField                   "Error"
#define notFoundField              "Not-Found"
#define searchField                 "Search"
#define searchResultsField       "Search-Results"

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
        appendField(request, (performFullTextSearch_?searchField:getDefinitionField), term_);
        
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
        error=numericValue(value, numValue, 16);
        if (error || (numValue!=transactionId_))
            error=errResponseMalformed;
    }
    else if (0==name.find(notFoundField))
        notFound_=true;
    else if (0==name.find(formatVersionField))
    {
        error=numericValue(value, numValue);
        if (!error)
            formatVersion_=numValue;
        else
            error=errResponseMalformed;
    }
    else if (0==name.find(resultsForField))
        resultsFor_=value;
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
    else if (0==name.find(searchResultsField))
    {
        error=numericValue(value, numValue);
        if (!error)
        {
            SearchResultsHandler* handler=new SearchResultsHandler();
            startPayload(handler, numValue);
            payloadType_=payloadSearchResults;
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
                            form->history().replaceForward(resultsFor_);
                            break;
                        
                        default:
                            assert(false);
                    }                    
                    form->setDisplayMode(MainForm::showDefinition);
                    form->synchronizeWithHistory();
                    form->update();
                }
            }
            if (searchResultsHandler_!=0)
            {
            }

            if (registering_ && !serverError_)
                app.preferences().serialNumberRegistered=true;
            
            if (notFound_)
                app.sendDisplayAlertEvent(definitionNotFoundAlert);
        }
        else
            handleServerError();
    }
    return error;        
}

#define SOCK_CONN_REFUSED 10061

void iPediaConnection::handleError(Err error)
{
    log()<<"handleError(), error code "<<error;
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

        case SOCK_CONN_REFUSED:
            alertId=cantConnectToServerAlert;
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
    switch (payloadType_)
    {
        case payloadDefinition:
            delete definitionParser_;
            definitionParser_=static_cast<DefinitionParser*>(releasePayloadHandler());
            break;
        
        case payloadSearchResults:
            delete searchResultsHandler_;
            searchResultsHandler_=static_cast<SearchResultsHandler*>(releasePayloadHandler());
            break;
            
        default:
            assert(false);
    }
    payloadType_=payloadNone;
    FieldPayloadProtocolConnection::notifyPayloadFinished();
}
