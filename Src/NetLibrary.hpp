#ifndef  __ARSLEXIS_PALMNETLIBRARY_HPP__
#define __ARSLEXIS_PALMNETLIBRARY_HPP__

#include <BaseTypes.hpp>
#include <NativeSocks.hpp>

namespace ArsLexis 
{

    class INetSocketAddress;

    class NetLibrary
#if defined(_PALM_OS)    
        :private Library
#endif        
    {
        bool closed_:1;
        bool libraryOpened_:1;
        
    public:
        
        NetLibrary();

        ~NetLibrary();

        enum {defaultConfig};

        status_t initialize(uint_t& ifError, uint_t configIndex=defaultConfig, ulong_t openFlags=0);

        bool closed() const 
        {return closed_;}

        status_t close(bool immediate=false);

        //status_t getSetting(ushort_t setting, void* value, ushort_t& valueLength) const;
        //status_t setSetting(ushort_t setting, const void* value, ushort_t valueLength);

        status_t getHostByName(const char_t* name, HostInfoBuffer& buffer, long timeout=evtWaitForever);

        status_t addrAToIN(const char_t* addr, INetSocketAddress& out);
        
        NativeSocket_t socketOpen(NativeSockAddrFamily_t  domain,  NativeSocketType_t type, int protocol, long timeout, status_t& error);
                
        int socketClose(NativeSocket_t socket, long  timeout, status_t& error);

        int socketShutdown(NativeSocket_t socket, int direction, long timeout, status_t& error) ;

        int socketSend(NativeSocket_t socket, void* bufP, uint_t bufLen, uint_t flags, void* toAddrP, uint_t toLen, long timeout, status_t& error);

        int socketReceive(NativeSocket_t socket, void* bufP, uint_t bufLen, uint_t flags, void* fromAddrP, uint_t* fromLen, long  timeout, status_t& error);
            
        int socketConnect(NativeSocket_t socket, const SocketAddr& sockAddrP, uint_t addrLen, long timeout, status_t& error);

        int select(uint_t width, NativeFDSet_t* readFDs, NativeFDSet_t *writeFDs, NativeFDSet_t *exceptFDs, long timeout, status_t& error);

        int socketOptionGet(NativeSocket_t socket, uint_t level, uint_t  option, void* optValueP, uint_t& optValueLen, long timeout, status_t& error);

        int socketOptionSet(NativeSocket_t socket, uint_t level, uint_t  option, void* optValueP, uint_t optValueLen, long timeout, status_t& error);
        
    };
    
}

#endif