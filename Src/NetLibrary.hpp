#ifndef  __ARSLEXIS_PALMNETLIBRARY_HPP__
#define __ARSLEXIS_PALMNETLIBRARY_HPP__

#include <BaseTypes.hpp>
#include <NativeSocks.hpp>

namespace ArsLexis 
{

    class INetSocketAddress;

    class NetLibrary

		: private Library
    {
        bool closed_:1;
        bool libraryOpened_:1;
        
    public:
        
        NetLibrary();
        
        ~NetLibrary();
        
        enum {
            defaultConfig
        };
        
        operator ushort_t() const;
        
        status_t initialize(ushort_t& ifError, ushort_t configIndex=defaultConfig, ulong_t openFlags=0);
        
        bool closed() const 
        {return closed_;}
        
         status_t close(bool immediate=false);
        
         //status_t getSetting(ushort_t setting, void* value, ushort_t& valueLength) const;
         //status_t setSetting(ushort_t setting, const void* value, ushort_t valueLength);
        
         status_t getHostByName(const char* name, HostInfoBufType* buffer, long  timeout=evtWaitForever);
        
         status_t addrAToIN(const char* addr, INetSocketAddress& out);
        
	
	NativeSocket_t socketOpen (
	        NetSocketAddrEnum domain,  
                NetSocketTypeEnum type,
                short protocol, 
                long timeout, status_t *errP);
                
        short socketClose (NativeSocket_t socket, long  timeout, status_t *errP);
        
        short socketShutdown (
                NativeSocket_t socket, 
                short direction,
                long  timeout, status_t *errP) ;

        short  socketSend (
                NativeSocket_t socket, 
                void *bufP, ushort_t bufLen,
                ushort_t flags, 
                void *toAddrP, ushort_t toLen,
                long timeout, status_t *errP) ;
        
        short socketReceive (
            NativeSocket_t socket, 
            void *bufP, 
            ushort_t bufLen, 
            ushort_t flags, 
            void *fromAddrP, 
            ushort_t  *fromLenP, 
            long  timeout, 
            status_t *errP);
            
        short socketConnect (
            NativeSocket_t  socket, 
            NativeSocketAddr_t *sockAddrP,
            short  addrLen, 
            long timeout, 
            status_t *errP) ;

        short select (
                ushort_t  width, 
                NativeFDSet_t *readFDs,
                NativeFDSet_t *writeFDs, 
                NativeFDSet_t *exceptFDs,
                long timeout, 
                status_t *errP) ;

        short socketOptionGet (
                NativeSocket_t socket, 
                ushort_t level, 
                ushort_t  option,
                void *optValueP, 
                ushort_t *optValueLenP,
                long  timeout, 
                ushort_t *errP);

    short socketOptionSet (
            NativeSocket_t socket, 
            ushort_t level, 
            ushort_t  option,
            void *optValueP, 
            ushort_t  optValueLen,
            long  timeout, 
            status_t *errP);
    };
    
}

#endif