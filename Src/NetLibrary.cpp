#include "NetLibrary.hpp"
#include "Application.hpp"

#define netLibName "Net.lib"

namespace ArsLexis
{

    NetLibrary::NetLibrary():
        closed_(false)
    {
    }
    
    Err NetLibrary::initialize(UInt16& ifError, UInt16 configIndex, UInt32 openFlags)
    {
        Err error=Library::initialize(netLibName, netCreator);
        if (!error)
        {
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
        return error;      
    }
    
    Err NetLibrary::close(Boolean immediate)
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
        return NetLibSettingGet(refNum(), setting, value, &valueLength);
    }
    
    Err NetLibrary::setSetting(UInt16 setting, const void* value, UInt16 valueLength)
    {
        return NetLibSettingSet(refNum(), setting, const_cast<void*>(value), valueLength);
    }

    Err NetLibrary::getHostByName(const char* name, NetHostInfoBufType* buffer, Int32 timeout)
    {
        Err error=errNone;
        NetHostInfoType* info=NetLibGetHostByName(refNum(), name, buffer, timeout, &error);
        return error;
    }

}