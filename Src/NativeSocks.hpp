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
    
    const int netFeatureUnimplemented=INT_MAX;

#if defined(_PALM_OS)

    typedef NetSocketAddrType NativeSocketAddr_t;
    typedef NetIPAddr NativeIPAddr_t;
    typedef NetSocketAddrINType NativeSockAddrIN_t;
    typedef NetFDSetType NativeFDSet_t;
    typedef NetSocketRef NativeSocket_t;
    typedef NetSocketLingerType NativeSocketLinger_t;
    typedef NetSocketAddrEnum NativeSockAddrFamily_t;
    typedef NetSocketTypeEnum NativeSocketType_t;

    const NativeSocket_t invalidSocket = -1;
    const NetSocketAddrEnum socketAddrINET = netSocketAddrINET;
    const NetSocketTypeEnum socketTypeStream = netSocketTypeStream;
    const NetSocketOptLevelEnum socketOptLevelSocket  = netSocketOptLevelSocket;
    const NetSocketOptLevelEnum socketOptLevelTCP = netSocketOptLevelTCP;
    const NetSocketOptEnum socketOptSockNonBlocking = netSocketOptSockNonBlocking;
    const NetSocketOptEnum socketOptSockLinger=netSocketOptSockLinger;
    const NetSocketOptEnum socketOptSockErrorStatus=netSocketOptSockErrorStatus;
    const NetSocketOptEnum socketOptTCPMaxSeg=netSocketOptTCPMaxSeg;

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
    
    const NativeSocket_t invalidSocket = INVALID_SOCKET;
    const short socketTypeStream = SOCK_STREAM;
    const short socketAddrINET = AF_INET;
    const int socketOptSockLinger = SO_LINGER;
    const int socketOptLevelSocket = SOL_SOCKET;
    const int socketOptLevelTCP = IPPROTO_TCP;
    const int  socketOptSockErrorStatus= SO_ERROR ;
    const int socketOptTCPMaxSeg=netFeatureUnimplemented;

// on CE 3.0 (Smartphone 2002/Pocket PC 2002) SD_SEND, SD_RECEIVE and SD_BOTH
// don't seem to be defined and docs don't say what they are
// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/wcesdkr/html/wcesdkrshutdown_function.asp
// C:\Program Files\Windows CE Tools\wce300\Pocket PC 2002\include\WINSOCK.H
// this might be not correct, defined after http://cryptopp.sourceforge.net/docs/ref5/socketft_8h-source.html

#ifndef SD_RECEIVE
#define SD_RECEIVE 0
#define SD_SEND 1
#define SD_BOTH 2
#endif
    const int netSocketDirOutput = SD_SEND;
    const int netSocketDirInput = SD_RECEIVE;
    const int netSocketDirBoth = SD_BOTH;
    //TODO: Move errors to ErrBase.h and correct return values returned 
    //in WinNetLibrary

    #define netToHostS ntohs
    #define hostToNetS htons
    #define netFDSet(n,p) FD_SET(n, p)
    #define netFDClr(n,p) FD_CLR(n, p)
    #define netFDIsSet(n,p) FD_ISSET(n,p)
    #define netFDZero(p) FD_ZERO(p)
    
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
        } portable;
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
        NativeIPAddr_t native;
    } ;
    

#if defined(_PALM_OS)

    class HostInfoBuffer {
        NetHostInfoBufType hostInfo_;
    public:
        NetHostInfoBufType& hostInfo() { return hostInfo_; };
        IPAddr getAddress() 
        {
             IPAddr  ipaddr;  
             ipaddr.ip=hostInfo_.address[0]; 
             return ipaddr;
        }
    };

#elif defined(_WIN32)	

    class HostInfoBuffer {
        struct hostent *hostInfo_;
    public:
        struct hostent& hostInfo()  { return *hostInfo_; }
        //TODO: correct apropriately - set s_addr
        IPAddr getAddress() {  
            IPAddr ret; 
            ret.ip = *(long*)hostInfo_->h_addr_list[0];
            return ret; 
        }
        void setHostInfo(struct hostent *hostInfo) { hostInfo_=hostInfo; }
    };

#endif

}

#if defined(_WIN32)	

    const ArsLexis::status_t netErrorClass = WSABASEERR;
    const ArsLexis::status_t netErrParamErr = WSAEINVAL;
    const ArsLexis::status_t netErrTimeout = WSAETIMEDOUT;
    const ArsLexis::status_t netErrSocketClosedByRemote = WSAECONNRESET;
    const ArsLexis::status_t netErrWouldBlock = WSAEWOULDBLOCK;
    const ArsLexis::status_t netErrSocketBusy = WSAEADDRINUSE;
    const ArsLexis::status_t netErrUnimplemented = WSAEOPNOTSUPP;
    
    const ArsLexis::status_t memErrorClass = 0x0100;
    const ArsLexis::status_t memErrNotEnoughSpace = memErrorClass | 2;
    
#endif

#endif //__ARSLEXIS_NATIVESOCKS_HPP__
