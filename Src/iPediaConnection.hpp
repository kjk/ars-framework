#ifndef __DEFINITION_REQUEST_CONNECTION_HPP__
#define __DEFINITION_REQUEST_CONNECTION_HPP__

#include <FieldPayloadProtocolConnection.hpp>
#include "LookupManager.hpp"

class DefinitionParser;

class iPediaConnection: public ArsLexis::FieldPayloadProtocolConnection
{
    LookupManager& lookupManager_;
    UInt32 transactionId_;
    ArsLexis::String term_;
    uint_t formatVersion_;
    ArsLexis::String resultsFor_;
    DefinitionParser* definitionParser_;
    
    enum StatusString
    {
        statusStringResolvingAddress,
        statusStringOpeningConnection,
        statusStringSendingRequest,
        statusStringWaitingForAnswer,
        statusStringRetrievingResponse
    };        
    
    class SearchResultsHandler: public PayloadHandler, private ArsLexis::NonCopyable
    {
        ArsLexis::String searchResults_;

    public:

        SearchResultsHandler()
        {}
        
        Err handleIncrement(const ArsLexis::String& text, ulong_t& length, bool finish)
        {
            if (finish)
                searchResults_.assign(text, 0, length);
            else
                length=0;                
            return errNone;
        }
        
        const ArsLexis::String& searchResults() const
        {return searchResults_;}
        
    };
    
    SearchResultsHandler* searchResultsHandler_;
    
    void prepareRequest();
    
    enum PayloadType 
    {
        payloadNone,
        payloadDefinition,
        payloadSearchResults
    };
    
    PayloadType payloadType_;

protected:

    Err notifyFinished();
    
    void handleError(Err error);
    
    Err handleField(const ArsLexis::String& name, const ArsLexis::String& value);
    
    void notifyPayloadFinished();
    
    Err notifyProgress();

    Err open();
    
public:

    Err enqueue();

    iPediaConnection(LookupManager& lm);
    
    ~iPediaConnection();
    
    void setTerm(const ArsLexis::String& term)
    {term_=term;}
    
    void setPerformFullTextSearch(bool value)
    {performFullTextSearch_=value;}

    void setRandom()
    {getRandom_=true;}
    
private:
    
    LookupManager::ServerError serverError_;

    uint_t notFound_:1;
    uint_t registering_:1;
    uint_t performFullTextSearch_:1;
    uint_t getRandom_:1;
    
};

#endif