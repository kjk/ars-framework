#ifndef __ARSLEXIS_RESOLVER_HPP__
#define __ARSLEXIS_RESOLVER_HPP__

#include "SocketAddress.hpp"
//#include <list>
#include <map>

namespace ArsLexis
{

    class NetLibrary;
    
    class SocketConnection;
    
    class Resolver
    {
/*    
        struct CacheEntry
        {
            String name;
            UInt32 address;

            CacheEntry():
                address(0)
            {}
            
            CacheEntry(const String& n, UInt32 a):
                name(n),
                address(a)
            {}
        };
        
        struct CacheEntryComparator
        {
            const String& name;
            
            bool operator ()(const CacheEntry& entry) const
            {return entry.name==name;}
            
            CacheEntryComparator(const String& n):
                name(n)
            {}
        };
        
        // Painful, but std::map<> doesn't compile with our Allocator (that's a mystery why), so we have to use std::list<>
        // Anyway this list probably won't ever contain more than few addresses, so there's no problem with performance
        typedef std::list<CacheEntry, Allocator<CacheEntry> > AddressCache_t;
        AddressCache_t cache_;
   */
        NetLibrary& netLib_;
        typedef std::map<String, UInt32> AddressCache_t;
        AddressCache_t cache_;
        
        enum {dnsAddressesCount_=2};
        UInt32 dnsAddresses_[dnsAddressesCount_];
             
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
        
       ~Resolver();
       
       void updateCacheEntry(const String& name, UInt32 address);
       
       Err resolveAndConnect(SocketConnection* connection, const String& address, UInt16 port=0);
       
       friend class ResolverConnection;

    private:
    
        void doResolveAndConnect(SocketConnection* connection, const String& name, UInt16 port, DNS_Choice choice);
        
    };
    
}

#endif    