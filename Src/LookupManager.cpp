#include "LookupManager.hpp"
#include "iPediaApplication.hpp"
#include "iPediaConnection.hpp"
#include <Graphics.hpp>
#include <Text.hpp>

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

        case memErrNotEnoughSpace:
            alertId=notEnoughMemoryAlert;
            break;            

    }
    iPediaApplication::sendDisplayAlertEvent(alertId);
}

void LookupManager::handleDefinition()
{
    switch (historyChange_)
    {
        case historyReplaceForward:
            history_.replaceAllNext(lastFoundTerm_);
            break;
        
        case historyMoveBack:
            history_.movePrevious(lastFoundTerm_);
            break;
        
        case historyMoveForward:
            history_.moveNext(lastFoundTerm_);
            break;
    }
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

bool LookupManager::lookupIfDifferent(const ArsLexis::String& term)
{
    using ArsLexis::equalsIgnoreCase;
    bool result=false;
    if (lastDefinition_.empty() || !equalsIgnoreCase(lastInputTerm(), term))
    {
        lookupTerm(term);
        result=true;
    }
    return result;
}

void LookupManager::lookupTerm(const ArsLexis::String& term)
{
    historyChange_=historyReplaceForward;
    iPediaConnection* conn=new iPediaConnection(*this);
    conn->setTerm(lastInputTerm_=term);
    resolver_.resolveAndConnect(conn, iPediaApplication::instance().server());
}

void LookupManager::lookupRandomTerm()
{
    historyChange_=historyReplaceForward;
    iPediaConnection* conn=new iPediaConnection(*this);
    conn->setRandom();
    resolver_.resolveAndConnect(conn, iPediaApplication::instance().server());
}

void LookupManager::search(const ArsLexis::String& expression)
{
    historyChange_=historyReplaceForward;
    iPediaConnection* conn=new iPediaConnection(*this);
    conn->setTerm(expression);
    conn->setPerformFullTextSearch(true);
    resolver_.resolveAndConnect(conn, iPediaApplication::instance().server());
}


void LookupManager::moveHistory(bool forward)
{
    if ((forward && history_.hasNext()) || (!forward && history_.hasPrevious()))
    {
        historyChange_=(forward?historyMoveForward:historyMoveBack);
        iPediaConnection* conn=new iPediaConnection(*this);
        conn->setTerm(lastInputTerm_=(forward?history_.nextTerm():history_.previousTerm()));
        resolver_.resolveAndConnect(conn, iPediaApplication::instance().server());
    }
}

void LookupManager::showProgress(ArsLexis::Graphics& graphics, const ArsLexis::Rectangle& bounds) const
{
    using ArsLexis::Graphics;
    using ArsLexis::Rectangle;
    using ArsLexis::Point;
    
    graphics.erase(bounds);
    Rectangle rect(bounds);
    rect.explode(2, 2, -4, -4);
    Graphics::FontSetter setFont(graphics, stdFont);
    uint_t length=statusText_.length();
    uint_t width=rect.width();
    const char* text=statusText_.c_str();
    graphics.charsInWidth(text, length, width);
    uint_t height=graphics.fontHeight();
    Point p(rect.x(), rect.y()+(rect.height()-height)/2);
    graphics.drawText(text, length, p);
    if (percentProgressDisabled!=percentProgress_)
    {
        assert(percentProgress_<=100);
        p.x+=width+2;
        char buffer[8];
        length=StrPrintF(buffer, " %hd%%", percentProgress_);
        width=rect.width()-width;
        graphics.charsInWidth(buffer, length, width);
        graphics.drawText(buffer, length, p);
    }
}
