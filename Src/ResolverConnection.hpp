#ifndef __ARSLEXIS_RESOLVER_CONNECTION_HPP__
#define __ARSLEXIS_RESOLVER_CONNECTION_HPP__

#include <Resolver.hpp>
#include <SimpleSocketConnection.hpp>
#include <list>

namespace ArsLexis
{

    class Resolver;

    class ResolverConnection: public SimpleSocketConnection
    {
        
        Resolver& resolver_;
        SocketConnection* nextConnection_;
        String address_;
        UInt16 port_;
        Resolver::DNS_Choice dnsChoice_;
        UInt16 requestId_;
        const char* data_;
        NetIPAddr resolvedAddress_;
        
        typedef std::list<String> Aliases_t;
        Aliases_t aliases_;
        
        
        void processAnswerRecord(const char*& data);
        
        String decodeName(const char*& data);
        
        void buildQuery();
    
    protected:
    
        void handleError(Err error);
        
        Err notifyFinished();
    
    public:

        ResolverConnection(Resolver& resolver, SocketConnection* nextConnection, const String& address, UInt16 port, Resolver::DNS_Choice dnsChoice);

        ~ResolverConnection();
        
        Err open();
        
    };
    
}

#endif
