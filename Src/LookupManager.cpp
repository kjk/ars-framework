#include "LookupManager.hpp"
#include "iPediaApplication.hpp"
#include "iPediaConnection.hpp"

Err LookupManager::initialize()
{
    UInt16 ifError;
    Err error=netLibrary_.initialize(ifError);
    if (errNone==error && errNone!=ifError)
        error=ifError;
    if (!error)
    {
        iPediaApplication& app=iPediaApplication::instance();
        lastDefinition_.setHyperlinkHandler(&app.hyperlinkHandler());        
        resolver_.initialize();
    }
    return error;
}

LookupManager::~LookupManager()
{
}

static const UInt16 serverErrorAlerts[]=
{   
    serverFailureAlert,
    unsupportedDeviceAlert,
    invalidAuthorizationAlert,
    malformedRequestAlert
};

void LookupManager::handleServerError(LookupManager::ServerError serverError)
{
    assert(LookupManager::serverErrorNone!=serverError);
    assert(LookupManager::serverErrorLast>=serverError);
    iPediaApplication::sendDisplayAlertEvent(serverErrorAlerts[serverError-1]);
}

void LookupManager::handleConnectionError(Err error)
{
    using ArsLexis::SocketConnection;
    UInt16 alertId=connectionErrorAlert;
    switch (error)
    {
        case SocketConnection::errResponseTooLong:
            alertId=definitionTooBigAlert;
            break;
            
        case SocketConnection::errResponseMalformed:
            alertId=malformedResponseAlert;
            break;
        
        case netErrTimeout:
            alertId=connectionTimedOutAlert;
            break;

    }
    iPediaApplication::sendDisplayAlertEvent(alertId);
}

void LookupManager::handleDefinition()
{
    switch (historyChange_)
    {
        case historyReplaceForward:
            history_.replaceAllNext(lastTerm_);
            break;
        
        case historyMoveBack:
            history_.movePrevious(lastTerm_);
            break;
        
        case historyMoveForward:
            history_.moveNext(lastTerm_);
            break;
    }
}

void LookupManager::handleList()
{
}

void LookupManager::handleLookupEvent(const EventType& event)
{
    switch (event.eType)
    {
        case iPediaApplication::appLookupStartedEvent:
            lookupInProgress_=true;
            break;
            
        case iPediaApplication::appLookupFinishedEvent:
            {
                lookupInProgress_=false;
                const LookupFinishedEventData& data=reinterpret_cast<const LookupFinishedEventData&>(event.data);
                switch (data.outcome)
                {
                    case data.outcomeError:
                        handleConnectionError(data.error);
                        break;
                    
                    case data.outcomeServerError:
                        handleServerError(data.serverError);
                        break;
                        
                    case data.outcomeNotFound:
                        iPediaApplication::sendDisplayAlertEvent(definitionNotFoundAlert);
                        break;
                        
                    case data.outcomeDefinition:
                        handleDefinition();
                        break;
                }                        
            }
            break;                    
    }
}

void LookupManager::lookupIfDifferent(const ArsLexis::String& term)
{
    if (history_.empty() || history_.currentTerm()!=term)
        lookupTerm(term);
}

void LookupManager::lookupTerm(const ArsLexis::String& term)
{
    historyChange_=historyReplaceForward;
    iPediaConnection* conn=new iPediaConnection(*this);
    conn->setTerm(term);
    resolver_.resolveAndConnect(conn, iPediaApplication::instance().server());
}

void LookupManager::moveHistory(bool forward)
{
    if ((forward && history_.hasNext()) || (!forward && history_.hasPrevious()))
    {
        historyChange_=(forward?historyMoveForward:historyMoveBack);
        iPediaConnection* conn=new iPediaConnection(*this);
        conn->setTerm(forward?history_.nextTerm():history_.previousTerm());
        resolver_.resolveAndConnect(conn, iPediaApplication::instance().server());
    }
}

