#ifndef __ARSLEXIS_RESOLVER_CONNECTION_HPP__
#define __ARSLEXIS_RESOLVER_CONNECTION_HPP__

#include "SimpleSocketConnection.hpp"
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
        bool useSecondaryDNS_;
        UInt16 requestId_;
        const char* data_;
        UInt32 resolvedAddress_;
        
        typedef std::list<String, Allocator<String> > Aliases_t;
        Aliases_t aliases_;
        
        
        void processAnswerRecord(const char*& data);
        
        String decodeName(const char*& data);
        
        void buildQuery();
    
    protected:
    
        void handleError(Err error);
        
        void finalize();
    
    public:

        ResolverConnection(Resolver& resolver, SocketConnection* nextConnection, const String& address, UInt16 port);

        ~ResolverConnection();
        
        void open();
        
    };
    
}

#endif
