#ifndef __DEFINITION_REQUEST_CONNECTION_HPP__
#define __DEFINITION_REQUEST_CONNECTION_HPP__

#include "FieldPayloadProtocolConnection.hpp"

class DefinitionParser;

class iPediaConnection: public ArsLexis::FieldPayloadProtocolConnection
{
    UInt32 transactionId_;
    ArsLexis::String term_;

    uint_t definitionNotFound_:1;
    uint_t registering_:1;
    uint_t formatVersion_;
    
    ArsLexis::String definitionForTerm_;
    DefinitionParser* definitionParser_;
    
    void prepareRequest();
    
    enum PayloadType 
    {
        payloadNone,
        payloadDefinition,
        payloadTermList
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
    
private:
    
    HistoryChange historyChange_;
    ServerError serverError_;
    
    void handleServerError();
    
};

#endif