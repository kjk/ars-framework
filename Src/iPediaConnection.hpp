#ifndef __DEFINITION_REQUEST_CONNECTION_HPP__
#define __DEFINITION_REQUEST_CONNECTION_HPP__

#include "FieldPayloadProtocolConnection.hpp"

class DefinitionParser;

class iPediaConnection: public ArsLexis::FieldPayloadProtocolConnection
{
    UInt32 transactionId_;
    ArsLexis::String term_;
    uint_t formatVersion_;
    ArsLexis::String resultsFor_;
    DefinitionParser* definitionParser_;
    
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

public:

    enum HistoryChange
    {
        historyMoveBack,
        historyMoveForward,
        historyReplaceForward
    };

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

    void open();
    
    iPediaConnection(ArsLexis::SocketConnectionManager& manager);
    
    ~iPediaConnection();
    
    void setTerm(const ArsLexis::String& term)
    {term_=term;}
    
    void setHistoryChange(HistoryChange hc)
    {historyChange_=hc;}
    
    void setPerformFullTextSearch(bool value)
    {performFullTextSearch_=value;}
    
private:
    
    HistoryChange historyChange_;
    ServerError serverError_;

    uint_t notFound_:1;
    uint_t registering_:1;
    uint_t performFullTextSearch_:1;
    
    
    void handleServerError();
    
};

#endif