#ifndef __DEFINITION_REQUEST_CONNECTION_HPP__
#define __DEFINITION_REQUEST_CONNECTION_HPP__

#include "SimpleSocketConnection.hpp"

class DefinitionParser;

class iPediaConnection: public ArsLexis::SimpleSocketConnection
{
    DefinitionParser* parser_;
    UInt32 transactionId_;
    ArsLexis::String term_;
    ArsLexis::String serialNumber_;

    UInt16 getCookie_:1;
    UInt16 inPayload_:1;
    UInt16 payloadIsError_:1;
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

    Err open(const ArsLexis::SocketAddress& address, Int32 timeout=evtWaitForever);
    
    iPediaConnection(ArsLexis::SocketConnectionManager& manager);
    
    ~iPediaConnection();
    
    void setTerm(const ArsLexis::String& term)
    {term_=term;}
    
    void setSerialNumber(const ArsLexis::String& serialNumber)
    {serialNumber_=serialNumber;}
    
};

#endif