#ifndef __ARSLEXIS_RESOLVER_HPP__
#define __ARSLEXIS_RESOLVER_HPP__

#include <SocketAddress.hpp>
#include <BaseTypes.hpp>
#include <Utility.hpp>

namespace ArsLexis
{
    class NetLibrary;

    status_t resolve(SocketAddress& out, NetLibrary& netLib, const String& address, ushort_t port=0, ulong_t timeout=evtWaitForever);
}

#endif    
