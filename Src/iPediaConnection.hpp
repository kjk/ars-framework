#ifndef __DEFINITION_REQUEST_CONNECTION_HPP__
#define __DEFINITION_REQUEST_CONNECTION_HPP__

#include "FieldPayloadProtocolConnection.hpp"
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
    
    class SearchResultsHandler: public FieldPayloadProtocolConnection::PayloadHandler
    {
        const ArsLexis::String* text_;
        ArsLexis::String searchResults_;
        uint_t startOffset_;

    public:

        SearchResultsHandler():
            text_(0),
            startOffset_(0)
        {}
        
        void initialize(const ArsLexis::String& payload, uint_t startOffset)
        {
            text_=&payload;
            startOffset_=startOffset;
        }
                
        Err handleIncrement(uint_t incrementEnd, bool finish)
        {
            assert(text_!=0);
            if (finish)
                searchResults_.assign(*text_, startOffset_, incrementEnd-startOffset_);
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

public:

    void open();
    
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