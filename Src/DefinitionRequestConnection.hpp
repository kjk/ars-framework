#ifndef __DEFINITION_REQUEST_CONNECTION_HPP__
#define __DEFINITION_REQUEST_CONNECTION_HPP__

#include "SimpleSocketConnection.hpp"
#include "DefinitionParser.hpp"

class DefinitionRequestConnection: public ArsLexis::SimpleSocketConnection
{
    DefinitionParser parser_;

protected:

    void reportProgress();
    
    void finalize();

public:

    DefinitionRequestConnection(ArsLexis::SocketConnectionManager& manager, const ArsLexis::String& request);
    
    ~DefinitionRequestConnection();
    
};

#endif