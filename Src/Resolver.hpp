#ifndef __ARSLEXIS_RESOLVER_HPP__
#define __ARSLEXIS_RESOLVER_HPP__

#include <SocketAddress.hpp>
#include <BaseTypes.hpp>
#include <Utility.hpp>

namespace ArsLexis
{
    class NetLibrary;
    class SocketAddress;

    status_t resolveFunc(SocketAddress& out, NetLibrary& netLib, const String& address, ushort_t port=0, ulong_t timeout=evtWaitForever);

    class Resolver : private NonCopyable
    {
        NetLibrary& netLib_;
    private:
        status_t blockingResolve(SocketAddress& out, const String& name, ushort_t port, ulong_t timeout);

    public:
        Resolver(NetLibrary& netLibrary);
       ~Resolver();
       status_t resolve(SocketAddress& out, const String& address, ushort_t port=0, ulong_t timeout=evtWaitForever);
    };    
}

#endif    
