#ifndef __ARSLEXIS_NATIVESOCKS_HPP__
#define __ARSLEXIS_NATIVESOCKS_HPP__

#if defined(_WIN32_WCE)
#include <winsock.h>
#elif defined(__PALMOS_H__)
#include <Library.hpp>
#endif


namespace ArsLexis 
{

#if defined(__PALMOS_H__)

    typedef NetSocketAddrType NativeSocketAddr_t;
    typedef NetIPAddr NativeIPAddr_t;
    typedef NetSocketAddrINType NativeSockAddrIN_t;
    typedef NetFDSetType NativeFDSet_t;
    typedef NetSocketRef NativeSocket_t;
    typedef NetSocketLingerType NativeSocketLinger_t;
    typedef NetSocketAddrEnum NativeSockAddrFamily_t;
    typedef NetSocketTypeEnum NativeSocketType_t;
    const NetSocketAddrEnum SocketAddrINET_c = netSocketAddrINET;
    const NetSocketTypeEnum SocketTypeStream_c = netSocketTypeStream;
    const NetSocketOptLevelEnum SocketOptLevelSocket_c  = netSocketOptLevelSocket;
    const NetSocketOptEnum SocketOptSockNonBlocking_c = netSocketOptSockNonBlocking;
    const NetSocketOptEnum SocketOptSockLinger_c=netSocketOptSockLinger;
    const NetSocketOptEnum SocketOptSockErrorStatus_c=netSocketOptSockErrorStatus;
    #define netToHostS(x) NetNToHS(x)
    #define hostToNetS(x) NetHToNS(x)

    class HostInfoBufType
    {
    	NetHostInfoBufType hostInfo_;
    	public:
 		NetHostInfoBufType& hostInfo() {	return hostInfo_; }
 		NativeIPAddr_t getAddress() {  return hostInfo_.address[0]; }
    };

#elif defined(_WIN32_WCE)	
    
    typedef struct sockaddr NativeSocketAddr_t;
    typedef struct in_addr NativeIPAddr_t;
    typedef struct sockaddr_in NativeSockAddrIN_t;
    typedef struct linger NativeSocketLinger_t;
    typedef fd_set NativeFDSet_t;
    typedef SOCKET NativeSocket_t;
    typedef int NativeSockAddrFamily_t;
    typedef int NativeSocketType_t;

    const short SocketTypeStream_c = SOCK_STREAM;
    const short SocketAddrINET_c = AF_INET;
    #define netToHostS ntohs
    #define hostToNetS htons
    #define netFDSet(n,p) FD_SET(n, p)
    #define netFDClr(n,p) FD_CLR(n, p)
    #define netFDIsSet(n,p) FD_ISSET(n,p)
    #define netFDZero(p) FD_ZERO(p)

    class HostInfoBufType
    {
    	struct hostent *hostInfo_;
    	public:
 			struct hostent& hostInfo() {	return *hostInfo_; }
			//TODO: correct apropriately - set s_addr
 			NativeIPAddr_t getAddress() {  in_addr ret; ret.s_addr=0; return ret; }
			void setHostInfo(struct hostent *hostInfo) { hostInfo_=hostInfo; }
    };
	// Maybe bettert idea to get rid of this stuff
	class Library
	{

	};
	
	#define evtWaitForever	-1
	
#else

#error "Define native sockets counterparts in NativeSocks.hpp before including Sockets.hpp"

#endif
    typedef struct 
    {
        short   onOff;
        short   time;
    } PortableSocketLinger_t;

    typedef union
    {
        PortableSocketLinger_t portable;
        NativeSocketLinger_t native;
    }CommonSocketLinger_t;

}

#endif //__ARSLEXIS_NATIVESOCKS_HPP__
