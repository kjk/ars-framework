#include <SysUtils.hpp>
#include <NetLibrary.hpp>
#include <SocketAddress.hpp>

#define netLibName "Net.lib"

namespace ArsLexis
{

    NetLibrary::NetLibrary():
        closed_(true),
        libraryLoaded_(false)
    {
    }
    
    status_t NetLibrary::initialize(uint_t& ifError, uint_t configIndex, ulong_t openFlags)
    {
        assert(closed_);
        status_t error=errNone;
        if (!libraryLoaded_)
            error=Library::initialize(netLibName, netCreator);
        if (errNone == error)
        {
            libraryLoaded_ = true;
            //! @bug NetLibOpenConf() returns netErrUnimplemented on Cobalt Simulator.
            //! To workaround it we call implemented NetLibOpen() instead...
            if (romVersionMajor()>5)
                error=NetLibOpen(refNum(), reinterpret_cast<UInt16*>(&ifError));
            else
                error=NetLibOpenConfig(refNum(), configIndex, openFlags, reinterpret_cast<UInt16*>(&ifError));
        }
        if (netErrAlreadyOpen == error)
            error = errNone;
        if (errNone == error)
            closed_ = false;
        return error;      
    }
    
    status_t NetLibrary::close(bool immediate)
    {
        assert(!closed_);
        status_t error = NetLibClose(refNum(), immediate);
        closed_ = true;
        return error;
    }

    NetLibrary::~NetLibrary()
    {
        if (!closed_)
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

    status_t NetLibrary::getHostByName(const char* name, HostInfoBuffer& buffer, long timeout)
    {
        assert(refNum());
        status_t error=errNone;
        NetHostInfoType* info=NetLibGetHostByName(refNum(), name, &buffer.hostInfo(), timeout, &error);
        return error;
    }

    status_t NetLibrary::addrAToIN(const char* addr, INetSocketAddress& out)
    {
        assert(refNum());
        status_t error=errNone;
        IPAddr a;
        a. ip=NetLibAddrAToIN(refNum(), addr);
        if (-1==a.ip)
            error=netErrParamErr;
        else
            out.setIpAddress(a);
        return error;
    }
    
    NativeSocket_t NetLibrary::socketOpen(NetSocketAddrEnum domain, NetSocketTypeEnum type, int protocol, long timeout, status_t& err)
    {
        return NetLibSocketOpen(*this, domain, type, protocol, timeout, &err);
    }
    
    int NetLibrary::socketShutdown(NativeSocket_t socket, int direction, long timeout, status_t& err) 
    {
        return NetLibSocketShutdown(*this, socket, direction, timeout, &err); 
    }
                
    int NetLibrary::socketSend(NativeSocket_t socket, void* bufP, uint_t bufLen, uint_t flags, void* toAddrP, uint_t toLen, long timeout, status_t& err) 
    {
        return NetLibSend(*this, socket, bufP, bufLen, flags, toAddrP, toLen, timeout, &err);
    }
    
    int NetLibrary::socketClose(NativeSocket_t socket, long timeout, status_t& err)
    {
        return NetLibSocketClose(*this, socket, timeout, &err);
    }
    
    int NetLibrary::socketReceive(NativeSocket_t socket, void* buf, uint_t bufLen, uint_t flags, void* fromAddrP, uint_t* fromLen, long  timeout, status_t& err)
    {
        err = errNone;
        int res = NetLibReceive(*this, socket, buf, bufLen, flags, fromAddrP, reinterpret_cast<UInt16*>(fromLen), timeout, &err);
        if (-1!=res)
        {
            assert(errNone==err);
        }
        return res;
    }

    int NetLibrary::socketConnect(NativeSocket_t  socket, const SocketAddr& sockAddr, uint_t addrLen, long timeout, status_t& err)
    {
        return NetLibSocketConnect(*this, socket, const_cast<NativeSocketAddr_t*>(&sockAddr.native), addrLen, timeout, &err);
    }
    
    int NetLibrary::select(uint_t width, NativeFDSet_t* readFDs, NativeFDSet_t* writeFDs, NativeFDSet_t* exceptFDs, long timeout, status_t& err) 
    {
        return NetLibSelect(*this, width, readFDs, writeFDs, exceptFDs, timeout, &err);
    }
    
    int NetLibrary::socketOptionGet(NativeSocket_t socket, uint_t level, uint_t option, void* optValueP, uint_t& optValueLen, long timeout, status_t& err)
    {
        return NetLibSocketOptionGet(*this, socket, level, option, optValueP, reinterpret_cast<UInt16*>(&optValueLen), timeout, &err);
    } 

    int NetLibrary::socketOptionSet(NativeSocket_t socket, uint_t level, uint_t option, void* optValueP, uint_t  optValueLen, long timeout, status_t& err)
    {
        return NetLibSocketOptionSet(*this, socket, level, option, optValueP, optValueLen, timeout, &err);  
    }
    
}