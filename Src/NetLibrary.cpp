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
    
    Err NetLibrary::initialize(UInt16& ifError, UInt16 configIndex, UInt32 openFlags)
    {
        assert(closed());
        Err error=errNone;
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
    
    Err NetLibrary::close(bool immediate)
    {
        assert(!closed());
        Err error=NetLibClose(refNum(), immediate);
        if (!error)
            closed_=true;
        return error;
    }

    NetLibrary::~NetLibrary()
    {
        if (!closed())
            close();
    }

    Err NetLibrary::getSetting(UInt16 setting, void* value, UInt16& valueLength) const
    {
        assert(refNum());
        return NetLibSettingGet(refNum(), setting, value, &valueLength);
    }
    
    Err NetLibrary::setSetting(UInt16 setting, const void* value, UInt16 valueLength)
    {
        assert(refNum());
        return NetLibSettingSet(refNum(), setting, const_cast<void*>(value), valueLength);
    }

    Err NetLibrary::getHostByName(const char* name, NetHostInfoBufType* buffer, Int32 timeout)
    {
        assert(refNum());
        Err error=errNone;
        NetHostInfoType* info=NetLibGetHostByName(refNum(), name, buffer, timeout, &error);
        return error;
    }

    Err NetLibrary::addrAToIN(const char* addr, INetSocketAddress& out)
    {
        assert(refNum());
        Err error=errNone;
        NetIPAddr ip=NetLibAddrAToIN(refNum(), addr);
        if (-1==ip)
            error=netErrParamErr;
        else
            out.setIpAddress(ip);
        return error;
    }
    
}