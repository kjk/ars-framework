#ifndef __DEFINITION_REQUEST_CONNECTION_HPP__
#define __DEFINITION_REQUEST_CONNECTION_HPP__

#include "SimpleSocketConnection.hpp"

class DefinitionParser;

class iPediaConnection: public ArsLexis::SimpleSocketConnection
{
    DefinitionParser* parser_;
    UInt32 transactionId_;
    ArsLexis::String term_;

    UInt16 inPayload_:1;
    UInt16 definitionNotFound_:1;
    UInt16 registering_:1;
    UInt16 formatVersion_:13;
    
    UInt16 payloadStart_;    
    UInt16 payloadLength_;
    UInt16 processedSoFar_;
    
    ArsLexis::String definitionForTerm_;
    
    Err processField(UInt16 fieldStart, UInt16 fieldEnd);
    
    void prepareRequest();
    
    void processLine(UInt16 start, UInt16 end);
    
    void processResponseIncrement(bool finish=false);
    
protected:

    void reportProgress();
    
    void finalize();
    
    void handleError(Err error);

public:

    enum HistoryChange
    {
        historyMoveBack,
        historyMoveForward,
        historyReplaceForward
    };

    void open();
    
    iPediaConnection(ArsLexis::SocketConnectionManager& manager);
    
    ~iPediaConnection();
    
    void setTerm(const ArsLexis::String& term)
    {term_=term;}
    
    void setHistoryChange(HistoryChange hc)
    {historyChange_=hc;}
    
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

private:
    
    HistoryChange historyChange_;
    ServerError serverError_;
    
    void handleServerError();
    
};

#endif