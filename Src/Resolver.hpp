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
        static Err validateAddress(const String& origAddress, String& validAddress, UInt16& port);
        
        Err blockingResolve(SocketAddress& out, const String& name, UInt16 port, UInt32 timeout);
        
    public:
        
        Resolver(NetLibrary& netLibrary);
        
       ~Resolver();
       
       void updateCacheEntry(const String& name, NetIPAddr address);
       
       Err resolve(SocketAddress& out, const String& address, UInt16 port=0, UInt32 timeout=evtWaitForever);
       
       friend class ResolverConnection;

    private:
    
    };
    
}

#endif    