#include <NetLibrary.hpp>
#include <SocketAddress.hpp>
#include <winsock.h>

namespace ArsLexis
{

    NetLibrary::NetLibrary():
        closed_(true),
        libraryOpened_(false)
    {
    }
    
    status_t NetLibrary::initialize(ushort_t& ifError, ushort_t configIndex, ulong_t openFlags)
    {
        assert(closed());
		
		WSADATA wsaData;
		WORD wVersionRequested = MAKEWORD( 1, 1 );
		status_t error = WSAStartup(wVersionRequested , &wsaData);
		
		if (!error)
            closed_=false;
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

    status_t NetLibrary::getHostByName(const char* name, HostInfoBufType* buffer, long timeout)
    {
        status_t error=0;
		struct hostent * hinfo = gethostbyname(name);
        //NetHostInfoType* info=NetLibGetHostByName(refNum(), name, buffer, timeout, &error);
		if(!hinfo)
			return WSAGetLastError();
		buffer->setHostInfo(hinfo);
		return 0;
    }

    status_t NetLibrary::addrAToIN(const char* addr, INetSocketAddress& out)
    {
        status_t error=0;
		NativeIPAddr_t ip;
        ip.s_addr = inet_addr(addr);
		//NetIPAddr ip=NetLibAddrAToIN(refNum(), addr);

        if (INADDR_NONE==ip.s_addr)
            error=-1;
        else
            out.setIpAddress(ip);

        return error;
    }

	NetLibrary::operator ushort_t() const
	{
		return 0;
	}
    
}