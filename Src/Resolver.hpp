#ifndef __ARSLEXIS_RESOLVER_HPP__
#define __ARSLEXIS_RESOLVER_HPP__

#include <SocketAddress.hpp>
#include <BaseTypes.hpp>
#include <Utility.hpp>
#include <map>

namespace ArsLexis
{

    class NetLibrary;
    
    class SocketConnection;
    
    class Resolver: private NonCopyable
    {

        NetLibrary& netLib_;
        typedef std::map<String, NetIPAddr> AddressCache_t;
        AddressCache_t cache_;
        
        enum {dnsAddressesCount_=2};
        NetIPAddr dnsAddresses_[dnsAddressesCount_];
             
        static Err validateAddress(const String& origAddress, String& validAddress, UInt16& port);
        
        void queryServerAddresses();
        
        void blockingResolveAndConnect(SocketConnection* connection, const String& name, UInt16 port);
        
    public:
        
        enum DNS_Choice
        {
            dnsPrimary,
            dnsSecondary
        };
        
        UInt32 dnsAddress(DNS_Choice choice=dnsPrimary);
    
        Resolver(NetLibrary& netLibrary);
        
        void initialize();
        
       ~Resolver();
       
       void updateCacheEntry(const String& name, NetIPAddr address);
       
       Err resolveAndConnect(SocketConnection* connection, const String& address, UInt16 port=0);
       
       friend class ResolverConnection;

    private:
    
        void doResolveAndConnect(SocketConnection* connection, const String& name, UInt16 port, DNS_Choice choice);
        
    };
    
}

#endif    