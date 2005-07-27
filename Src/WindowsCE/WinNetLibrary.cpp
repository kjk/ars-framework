#include <NetLibrary.hpp>
#include <SocketAddress.hpp>

NetLibrary::NetLibrary():
closed_(true)
{
}

status_t NetLibrary::initialize(uint_t& ifError, uint_t configIndex, ulong_t openFlags)
{
    assert(closed());
    
    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD( 1, 1 );
    status_t error = WSAStartup(wVersionRequested , &wsaData);
    ifError = errNone;    
    if (!error)
        closed_ = false;
    return error;      
}

status_t NetLibrary::close(bool immediate)
{
    
    assert(!closed());
    status_t error = WSACleanup();
    if (!error)
        closed_=true;
    return error;
}

NetLibrary::~NetLibrary()
{
    if (!closed())
        close();
}

status_t NetLibrary::getHostByName(const char* name, HostInfoBuffer& buffer, long timeout)
{
    struct hostent * hinfo = gethostbyname(name);

	//NetHostInfoType* info=NetLibGetHostByName(refNum(), name, buffer, timeout, &error);

	if(!hinfo)
        return WSAGetLastError();

    buffer.setHostInfo(hinfo);
    return errNone;
}

status_t NetLibrary::addrAToIN(const char* addr, INetSocketAddress& out)
{
    IPAddr ip;
    ip.ip= inet_addr(addr);

	//NetIPAddr ip=NetLibAddrAToIN(refNum(), addr);
    
    if (INADDR_NONE==ip.ip)
        return E_INVALIDARG;

	out.setIpAddress(ip);
    return errNone;
}

/*NetLibrary::operator ushort_t() const
{
    return 0;
}
*/

NativeSocket_t NetLibrary::socketOpen(NativeSockAddrFamily_t  domain,  NativeSocketType_t type, int protocol, long timeout, status_t& error)
{
    error = errNone;
    SOCKET sock=socket(domain, type, protocol);
    if (sock == INVALID_SOCKET)
        error=WSAGetLastError();
    //return values exactly the same as for NetSocketOpen
    return sock;
}

int NetLibrary::socketClose(NativeSocket_t socket, long  timeout, status_t& error)
{
    error = errNone;
    if (closesocket(socket))
    {
        error=WSAGetLastError();
        return -1;
    }
    return 0;
}

int NetLibrary::socketShutdown(NativeSocket_t socket, int direction, long timeout, status_t& error)
{
    // for whatever reason doing shutdown(direction=1) causes ipedia server
    // to break (i.e. nothing is returned).
    /*error = shutdown(socket, direction); */
    /* if(error)
    {
        error=WSAGetLastError();
        return -1;
    }*/
    return 0;    
}

int NetLibrary::socketSend(NativeSocket_t socket, void* bufP, uint_t bufLen, uint_t flags, void* toAddrP, uint_t toLen, long timeout, status_t& error)
{
    error = errNone;
    int res = sendto (socket, (const char*) bufP, bufLen, 0,(const struct sockaddr *) toAddrP, toLen);
    if (SOCKET_ERROR == res)
        error = WSAGetLastError();
    return res;
}

int NetLibrary::socketReceive(NativeSocket_t socket, void* bufP, uint_t bufLen, uint_t flags, void* fromAddrP, uint_t* fromLen, long  timeout, status_t& error)
{
    error = errNone;
    int res = recvfrom(socket, (char*) bufP, bufLen, 0, (struct sockaddr *) fromAddrP, (int*)fromLen);
    if (SOCKET_ERROR == res)
    {
        error = WSAGetLastError();
        return 0;
    }
    return res;
}

int NetLibrary::socketConnect(NativeSocket_t socket, const SocketAddr& sockAddrP, uint_t addrLen, long timeout, status_t& error)
{
    error = errNone;
    if (connect (socket, &sockAddrP.native, addrLen) == SOCKET_ERROR)
    {
        error = WSAGetLastError();
        return -1;
    }
    return 0;
}

int NetLibrary::socketOptionGet(NativeSocket_t socket, uint_t level, uint_t  option, void* optValueP, uint_t& optValueLen, long timeout, status_t& error)
{
    error = errNone;
    if (option == socketOptSockErrorStatus)
    {
        //if (error=WSAGetLastError()) return -1;
        return 0;
    }
    if (getsockopt(socket, level, option, (char*)optValueP, (int*)&optValueLen)== SOCKET_ERROR)
    {
        error = WSAGetLastError();
        return -1;
    }
    return 0;
}

int NetLibrary::socketOptionSet(NativeSocket_t socket, uint_t level, uint_t  option, void* optValueP, uint_t optValueLen, long timeout, status_t& error)
{
    error = errNone;
    if (setsockopt(socket, level, option, (const char*) optValueP, optValueLen) == SOCKET_ERROR)
    {
        error = WSAGetLastError();
        return -1;
    }
    return 0;
}

int NetLibrary::select(uint_t width, NativeFDSet_t* readFDs, NativeFDSet_t *writeFDs, NativeFDSet_t *exceptFDs, long timeout, status_t& error)
{
    error = errNone;
    timeval to;
    to.tv_usec = timeout * 1000;
    to.tv_sec = 0;
    const timeval* pto = &to;
    if (evtWaitForever == timeout)
        pto = NULL;
    int res = ::select(width, readFDs, writeFDs, exceptFDs, pto); 
    if (res == SOCKET_ERROR)
    {
        error = WSAGetLastError();
        return -1;            
    }
    return res;
}
