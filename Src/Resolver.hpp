#ifndef __ARSLEXIS_RESOLVER_HPP__
#define __ARSLEXIS_RESOLVER_HPP__

#include <SocketAddress.hpp>
#include <BaseTypes.hpp>
#include <NativeSocks.hpp>
#include <Utility.hpp>
#include <map>

namespace ArsLexis
{

    class NetLibrary;
    
    class SocketConnection;
    
    class Resolver: private NonCopyable
    {

        NetLibrary& netLib_;
        
        typedef std::map<String, IPAddr> AddressCache_t;
        
    public:        
    
//        typedef std::pair<String, NetIPAddr> CacheEntry_t;
        
    private:
    
//        typedef std::list<CacheEntry_t> AddressCache_t;
        
        AddressCache_t cache_;
        static status_t validateAddress(const String& origAddress, String& validAddress, ushort_t& port);
        
        status_t blockingResolve(SocketAddress& out, const String& name, ushort_t port, ulong_t timeout);
        
    public:
        
        Resolver(NetLibrary& netLibrary);
        
       ~Resolver();
       
       void updateCacheEntry(const String& name, unsigned long address);
       
       status_t resolve(SocketAddress& out, const String& address, ushort_t port=0, ulong_t timeout=evtWaitForever);
       
       friend class ResolverConnection;

    private:
    
    };
    
}

#endif    