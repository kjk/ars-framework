#ifndef __LOOKUP_MANAGER_HPP__
#define __LOOKUP_MANAGER_HPP__

#include <SocketConnection.hpp>
#include <Utility.hpp>
#include "Definition.hpp"

namespace ArsLexis
{
    class Graphics;
    class Rectangle;
}    

class LookupHistory;

class LookupManager: private ArsLexis::NonCopyable
{
    LookupHistory& history_;
    ArsLexis::SocketConnectionManager connectionManager_;
    Definition::Elements_t lastDefinitionElements_;

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

    ArsLexis::String statusText_;
    uint_t percentProgress_;    
    
    
    
public:

    enum ServerError
    {
        serverErrorNone,
        serverErrorFailure,
        serverErrorFirst=serverErrorFailure,
        serverErrorUnsupportedDevice,
        serverErrorInvalidAuthorization,
        serverErrorMalformedRequest,
        serverErrorTrialExpired,
        serverErrorLast=serverErrorTrialExpired
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

    LookupManager(LookupHistory& history):
        history_(history),
        historyChange_(historyMoveForward),
        percentProgress_(percentProgressDisabled),
        lookupInProgress_(false)
    {}
    
    ~LookupManager();

    ArsLexis::SocketConnectionManager& connectionManager()
    {return connectionManager_;}    

    const ArsLexis::String& lastSearchResults() const
    {return lastSearchResults_;}
    
    bool lookupInProgress() const
    {return lookupInProgress_;}

    void showProgress(ArsLexis::Graphics& graphics, const ArsLexis::Rectangle& bounds) const;

    void handleLookupEvent(const EventType& event);

    Definition::Elements_t& lastDefinitionElements() 
    {return lastDefinitionElements_;}
    
    void search(const ArsLexis::String& expression);

    void lookupTerm(const ArsLexis::String& term);

    void lookupRandomTerm();

    //! @return @c true if lookup is started, @c false otherwise.
    bool lookupIfDifferent(const ArsLexis::String& term);

    void moveHistory(bool forward=false);

    const ArsLexis::String& lastSearchExpression() const
    {return lastSearchExpression_;}

    const ArsLexis::String& lastInputTerm() const
    {return lastInputTerm_;}
    
    void handleLookupFinishedInForm(const LookupFinishedEventData& data);
    
    void checkArticleCount();
    
private:
    
    void handleServerError(ServerError serverError);
    
    void handleDefinitionMissing();
    
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
    
    void setLastInputTerm(const ArsLexis::String& lit)
    {lastInputTerm_=lit;}
    
    void setLastFoundTerm(const ArsLexis::String& t)
    {lastFoundTerm_=t;}
    
    void setLastSearchExpression(const ArsLexis::String& se)
    {lastSearchExpression_=se;}
    
    friend class iPediaConnection;

    HistoryChange historyChange_:7; // I don't remember how many bits it needs. I assume 7 will do.
    bool lookupInProgress_:1;
    
};

#endif