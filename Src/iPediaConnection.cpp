#include "iPediaConnection.hpp"
#include "iPediaApplication.hpp"
#include "DefinitionParser.hpp"
#include <SysUtils.hpp>
#include <DeviceInfo.hpp>
#include "LookupManager.hpp"

using namespace ArsLexis;

iPediaConnection::iPediaConnection(LookupManager& lookupManager):
    FieldPayloadProtocolConnection(lookupManager.connectionManager()),
    lookupManager_(lookupManager),
    transactionId_(random((UInt32)-1)),
    formatVersion_(0),
    definitionParser_(0),
    searchResultsHandler_(0),
    payloadType_(payloadNone),
    serverError_(LookupManager::serverErrorNone),
    notFound_(false),
    registering_(false),
    performFullTextSearch_(false),
    getRandom_(false)
{
}

iPediaConnection::~iPediaConnection()
{
    delete definitionParser_;
    delete searchResultsHandler_;
}

#define protocolVersion "1"

#define transactionIdField      "Transaction-ID"
#define protocolVersionField    "Protocol-Version"
#define clientVersionField      "Client-Version"
#define getCookieField          "Get-Cookie"
#define getDefinitionField      "Get-Definition"
#define getRandomDefField       "Get-Random-Definition"
#define cookieField             "Cookie"
#define registerField           "Register"
#define formatVersionField      "Format-Version"
#define resultsForField         "Results-For"
#define definitionField         "Definition"
#define errorField              "Error"
#define notFoundField           "Not-Found"
#define searchField             "Search"
#define searchResultsField      "Search-Results"

void iPediaConnection::prepareRequest()
{
    iPediaApplication& app=iPediaApplication::instance();
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

    if (getRandom_)
    {
        assert(term_.empty());
        appendField(request, getRandomDefField);
    }
    request+='\n';
    setRequest(request); 
}

void iPediaConnection::open()
{
    String status;
    getResource(connectionStatusStrings, statusStringOpeningConnection, status);
    lookupManager_.setStatusText(status);
    lookupManager_.setPercentProgress(LookupManager::percentProgressDisabled);
    Application::sendEvent(iPediaApplication::appLookupStartedEvent);
    
    prepareRequest();
    SimpleSocketConnection::open();
}

Err iPediaConnection::notifyProgress()
{
    Err error=FieldPayloadProtocolConnection::notifyProgress();
    if (!error)
    {
        String status;
        StatusString index=statusStringSendingRequest;
        if (!sending())
            index=(response().empty()?statusStringWaitingForAnswer:statusStringRetrievingResponse);
        getResource(connectionStatusStrings, index, status);
        lookupManager_.setStatusText(status);
        uint_t progress=LookupManager::percentProgressDisabled;
        if (inPayload())
            progress=((payloadLength()-payloadLengthLeft())*100L)/payloadLength();
        lookupManager_.setPercentProgress(progress);
        Application::sendEvent(iPediaApplication::appLookupProgressEvent);
    }
    return error;
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
        iPediaApplication& app=iPediaApplication::instance();
        if (value.length()>iPediaApplication::Preferences::cookieLength)
            error=errResponseMalformed;
        else
            app.preferences().cookie=value;
    }
    else if (0==name.find(errorField))
    {
        error=numericValue(value, numValue);
        if (!error)
        {
            if (numValue>=LookupManager::serverErrorFirst && numValue<=LookupManager::serverErrorLast)
                serverError_=static_cast<LookupManager::ServerError>(numValue);
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
        LookupManager::LookupFinishedEventData data;
        if (!serverError_)
        {
            iPediaApplication& app=iPediaApplication::instance();
            if (definitionParser_!=0)
            {
                definitionParser_->updateDefinition(lookupManager_.lastDefinition());
                lookupManager_.setLastFoundTerm(resultsFor_);
                data.outcome=data.outcomeDefinition;
            }
            if (searchResultsHandler_!=0)
            {
                lookupManager_.setLastSearchResults(searchResultsHandler_->searchResults());
                lookupManager_.setLastSearchExpression(resultsFor_);
                data.outcome=data.outcomeList;
            }

            if (registering_ && !serverError_)
                app.preferences().serialNumberRegistered=true;
            
            if (notFound_)
                data.outcome=data.outcomeNotFound;
        }
        else
        {
            data.outcome=data.outcomeServerError;
            data.serverError=serverError_;
        }
        Application::sendEvent(iPediaApplication::appLookupFinishedEvent, data);               
    }
    return error;        
}

void iPediaConnection::handleError(Err error)
{
    log()<<"handleError(): error code "<<error;
    LookupManager::LookupFinishedEventData data(LookupManager::LookupFinishedEventData::outcomeError, error);
    Application::sendEvent(iPediaApplication::appLookupFinishedEvent, data);
    SimpleSocketConnection::handleError(error);
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

