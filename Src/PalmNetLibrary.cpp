#include <NetLibrary.hpp>
#include <SocketAddress.hpp>
#include <Application.hpp>

#define netLibName "Net.lib"

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
        status_t error=errNone;
        if (!libraryOpened_)
            error=Library::initialize(netLibName, netCreator);
        if (!error)
        {
            libraryOpened_=true;
            Application& app=Application::instance();
            //! @bug NetLibOpenConf() returns netErrUnimplemented on Cobalt Simulator.
            //! To workaround it we call implemented NetLibOpen() instead...
            if (app.romVersionMajor()>5)
                error=NetLibOpen(refNum(), &ifError);
            else
                error=NetLibOpenConfig(refNum(), configIndex, openFlags, &ifError);
        }
        if (netErrAlreadyOpen==error)
            error=errNone;
        if (!error)
            closed_=false;
        return error;      
    }
    
    status_t NetLibrary::close(bool immediate)
    {
        assert(!closed());
        status_t error=NetLibClose(refNum(), immediate);
        if (!error)
            closed_=true;
        return error;
    }

    NetLibrary::~NetLibrary()
    {
        if (!closed())
            close();
    }

    /*status_t NetLibrary::getSetting(ushort_t setting, void* value, ushort_t& valueLength) const
    {
        assert(refNum());
        return NetLibSettingGet(refNum(), setting, value, &valueLength);
    }
    
    status_t NetLibrary::setSetting(ushort_t setting, const void* value, ushort_t valueLength)
    {
        assert(refNum());
        return NetLibSettingSet(refNum(), setting, const_cast<void*>(value), valueLength);
    }*/

    status_t NetLibrary::getHostByName(const char* name, HostInfoBufType* buffer, long timeout)
    {
        assert(refNum());
        status_t error=errNone;
        NetHostInfoType* info=NetLibGetHostByName(refNum(), name,&( buffer->hostInfo()), timeout, &error);
        return error;
    }

    status_t NetLibrary::addrAToIN(const char* addr, INetSocketAddress& out)
    {
        assert(refNum());
        status_t error=errNone;
        NetIPAddr ip=NetLibAddrAToIN(refNum(), addr);
        if (-1==ip)
            error=netErrParamErr;
        else
            out.setIpAddress(ip);
        return error;
    }
    
     NetLibrary::operator ushort_t() const
    {
    	return Library::refNum();
    }

    NativeSocket_t NetLibrary::socketOpen (
	        NetSocketAddrEnum domain,  
                NetSocketTypeEnum type,
                short protocol, 
                long timeout, 
                status_t *errP)
    {
         return NetLibSocketOpen(*this, domain, type, protocol, timeout, errP);       
    }
    
    short  NetLibrary::socketShutdown (
                NativeSocket_t socket, 
                short direction,
                long  timeout, 
                status_t *errP) 
    {
        return NetLibSocketShutdown(*this, socket, direction, timeout, errP); 
    }
                
    short  NetLibrary::socketSend (
                NativeSocket_t socket, 
                void *bufP, ushort_t bufLen,
                ushort_t flags, 
                void *toAddrP, ushort_t toLen,
                long timeout, status_t *errP) 
    {
        return NetLibSend(*this, socket, bufP, bufLen, flags, toAddrP, toLen, timeout, errP);
    }
    
    short NetLibrary::socketClose (NativeSocket_t socket, long  timeout, status_t *errP)
    {
        return NetLibSocketClose(*this, socket, timeout, errP);
    }
    
    short  NetLibrary::socketReceive (
            NativeSocket_t socket, 
            void *bufP, 
            ushort_t bufLen, 
            ushort_t flags, 
            void *fromAddrP, 
            ushort_t  *fromLenP, 
            long  timeout, 
            status_t *errP)
    {
        return NetLibReceive(*this, socket, bufP, bufLen, flags, 0, 0, timeout, errP);
    }

    short NetLibrary::socketConnect (
            NativeSocket_t  socket, 
            NativeSocketAddr_t * sockAddrP,
            short  addrLen, 
            long timeout, 
            status_t *errP)
    {
        return NetLibSocketConnect(*this, socket, sockAddrP, addrLen, timeout, errP);
    }
    short NetLibrary::select (
                ushort_t  width, 
                NativeFDSet_t *readFDs,
                NativeFDSet_t *writeFDs, 
                NativeFDSet_t *exceptFDs,
                long timeout, 
                status_t *errP) 
    {
        return NetLibSelect(*this, width, readFDs, writeFDs, exceptFDs, timeout, errP);
    }
    
    short NetLibrary::socketOptionGet (
        NativeSocket_t socket, 
        ushort_t level, 
        ushort_t  option,
        void *optValueP, 
        ushort_t *optValueLenP,
        long  timeout, 
        ushort_t *errP)
   {
        return NetLibSocketOptionGet(*this, socket, level, option, optValueP, optValueLenP, timeout, errP);
   } 

   short  NetLibrary::socketOptionSet(
            NativeSocket_t socket, 
            ushort_t level, 
            ushort_t  option,
            void *optValueP, 
            ushort_t  optValueLen,
            long  timeout, 
            status_t *errP)
    {
        return NetLibSocketOptionSet(*this, socket, level, option, optValueP, optValueLen, timeout, errP);  
    }
}