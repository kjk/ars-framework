#ifndef __LOOKUP_MANAGER_HPP__
#define __LOOKUP_MANAGER_HPP__

#include "LookupHistory.hpp"
#include "SocketConnection.hpp"
#include "NetLibrary.hpp"
#include "Resolver.hpp"
#include "Definition.hpp"

namespace ArsLexis
{
    class Graphics;
    class Rectangle;
}    

class LookupManager
{
    LookupHistory history_;
    ArsLexis::NetLibrary netLibrary_;
    ArsLexis::SocketConnectionManager connectionManager_;
    ArsLexis::Resolver resolver_;
    Definition lastDefinition_;

    ArsLexis::String lastInputTerm_;
    ArsLexis::String lastFoundTerm_;
    ArsLexis::String lastSearchExpression_;
    ArsLexis::String lastSearchResults_;
    
    enum HistoryChange
    {
        historyMoveBack,
        historyMoveForward,
        historyReplaceForward
    };

    HistoryChange historyChange_;    
    
    ArsLexis::String statusText_;
    uint_t percentProgress_;    
    uint_t lookupInProgress_:1;
    
public:

    Err initialize();

    enum ServerError
    {
        serverErrorNone,
        serverErrorFailure,
        serverErrorFirst=serverErrorFailure,
        serverErrorUnsupportedDevice,
        serverErrorInvalidAuthorization,
        serverErrorMalformedRequest,
        serverErrorLast=serverErrorMalformedRequest
    };
        
    struct LookupFinishedEventData
    {
        enum Outcome
        {
            outcomeNothing,
            outcomeDefinition,
            outcomeList,
            outcomeError,
            outcomeServerError,
            outcomeNotFound
        } outcome;
        
        union
        {
            Err error;
            ServerError serverError;
        };        
        
        LookupFinishedEventData(Outcome o=outcomeNothing, Err err=errNone):
            outcome(o),
            error(err)
        {}
    };

    explicit LookupManager():
        connectionManager_(netLibrary_),
        resolver_(netLibrary_),
        historyChange_(historyMoveForward),
        percentProgress_(percentProgressDisabled),
        lookupInProgress_(false)
    {
    }
    
    ~LookupManager();

    ArsLexis::SocketConnectionManager& connectionManager()
    {return connectionManager_;}    

    const LookupHistory& history() const
    {return history_;}
    
    const ArsLexis::String& lastSearchResults() const
    {return lastSearchResults_;}
    
    bool lookupInProgress() const
    {return lookupInProgress_;}

    void showProgress(ArsLexis::Graphics& graphics, const ArsLexis::Rectangle& bounds) const;
    
    void handleLookupEvent(const EventType& event);
    
    Definition& lastDefinition() 
    {return lastDefinition_;}
    
    void lookupTerm(const ArsLexis::String& term);
    
    void lookupIfDifferent(const ArsLexis::String& term);
    
    void moveHistory(bool forward=false);
    
    const ArsLexis::String& lastSearchExpression() const
    {return lastSearchExpression_;}
    
    const ArsLexis::String& lastInputTerm() const
    {return lastInputTerm_;}
    
private:
    
    void handleServerError(ServerError serverError);
    
    void handleConnectionError(Err error);
    
    void handleDefinition();
    
    void handleList();
    
    void setStatusText(const ArsLexis::String& text)
    {statusText_=text;}
    
    enum {percentProgressDisabled=(uint_t)-1};
    
    void setPercentProgress(uint_t progress)
    {percentProgress_=progress;}
    
    void setLastSearchResults(const ArsLexis::String& sr)
    {lastSearchResults_=sr;}
    
    void setLastFoundTerm(const ArsLexis::String& t)
    {lastFoundTerm_=t;}
    
    void setLastSearchExpression(const ArsLexis::String& se)
    {lastSearchExpression_=se;}
    
    friend class iPediaConnection;

};

#endif