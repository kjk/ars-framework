#ifndef __ARSLEXIS_NATIVESOCKS_HPP__
#define __ARSLEXIS_NATIVESOCKS_HPP__

#if defined(_WIN32_WCE)
#include <winsock.h>
#elif defined(_PALM_OS)
#include <Library.hpp>
#endif
#include <BaseTypes.hpp>

namespace ArsLexis 
{

#if defined(_PALM_OS)

    typedef NetSocketAddrType NativeSocketAddr_t;
    typedef NetIPAddr NativeIPAddr_t;
    typedef NetSocketAddrINType NativeSockAddrIN_t;
    typedef NetFDSetType NativeFDSet_t;
    typedef NetSocketRef NativeSocket_t;
    typedef NetSocketLingerType NativeSocketLinger_t;
    typedef NetSocketAddrEnum NativeSockAddrFamily_t;
    typedef NetSocketTypeEnum NativeSocketType_t;
    const NetSocketAddrEnum socketAddrINET = netSocketAddrINET;
    const NetSocketTypeEnum socketTypeStream = netSocketTypeStream;
    const NetSocketOptLevelEnum socketOptLevelSocket  = netSocketOptLevelSocket;
    const NetSocketOptEnum socketOptSockNonBlocking = netSocketOptSockNonBlocking;
    const NetSocketOptEnum socketOptSockLinger=netSocketOptSockLinger;
    const NetSocketOptEnum socketOptSockErrorStatus=netSocketOptSockErrorStatus;
    #define netToHostS(x) NetNToHS(x)
    #define hostToNetS(x) NetHToNS(x)

#elif defined(_WIN32_WCE)	
    
    typedef struct sockaddr NativeSocketAddr_t;
    typedef struct in_addr NativeIPAddr_t;
    typedef struct sockaddr_in NativeSockAddrIN_t;
    typedef struct linger NativeSocketLinger_t;
    typedef fd_set NativeFDSet_t;
    typedef SOCKET NativeSocket_t;
    typedef int NativeSockAddrFamily_t;
    typedef int NativeSocketType_t;

    const short socketTypeStream = SOCK_STREAM;
    const short socketAddrINET = AF_INET;
    const int socketOptSockLinger = SO_LINGER;
    const int socketOptLevelSocket = SOL_SOCKET;
    const int  socketOptSockErrorStatus= SO_ERROR ;
    const int netSocketDirOutput = 1;
    const status_t netErrorClass = 0x1200;
    const status_t netErrParamErr = netErrorClass | 4;
    const status_t netErrTimeout = netErrorClass | 18;
    const status_t netErrSocketClosedByRemote = netErrorClass | 20;
    const status_t netErrWouldBlock = netErrorClass | 47;
    const status_t netErrSocketBusy = netErrorClass | 9;
    #define netToHostS ntohs
    #define hostToNetS htons
    #define netFDSet(n,p) FD_SET(n, p)
    #define netFDClr(n,p) FD_CLR(n, p)
    #define netFDIsSet(n,p) FD_ISSET(n,p)
    #define netFDZero(p) FD_ZERO(p)
    
    // Maybe bettert idea to get rid of this stuff
    class Library
    {
    };

    #define evtWaitForever	-1
	
#else

# error "Define native sockets counterparts in NativeSocks.hpp before including Sockets.hpp"

#endif

    union SocketLinger
    {
        struct 
        {
            short   onOff;
            short   time;
        };
        NativeSocketLinger_t native;
    };
    
    union SocketAddr
    {
        struct
        {
    	    signed short  family;
    	    unsigned char data[14];
    	 };
        NativeSocketAddr_t native;
    };
    
    union IPAddr
    {
        unsigned long ip;
        NativeIPAddr_t navtive;
    } ;
    

#if defined(_PALM_OS)

    class HostInfoBuffer {
        NetHostInfoBufType hostInfo_;
    public:
        NetHostInfoBufType& hostInfo() {	return hostInfo_; };
        IPAddr getAddress() 
        {
             IPAddr  ipaddr;  
             ipaddr.ip=hostInfo_.address[0]; 
             return ipaddr;
        }
    };

#elif defined(_WIN32_WCE)	
    
    class HostInfoBuffer {
        struct hostent *hostInfo_;
    public:
        struct hostent& hostInfo()  { return *hostInfo_; }
        //TODO: correct apropriately - set s_addr
        IPAddr getAddress() {  IPAddr ret; ret.ip=0; return ret; }
        void setHostInfo(struct hostent *hostInfo) { hostInfo_=hostInfo; }
    };

#endif

}

#endif //__ARSLEXIS_NATIVESOCKS_HPP__
