
#include "iPediaApplication.hpp"
#include "SysUtils.hpp"
#include "ipedia_Rsc.h"


IMPLEMENT_APPLICATION_INSTANCE(appFileCreator)

using namespace ArsLexis;

iPediaApplication::iPediaApplication(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags):
    Application(cmd, cmdPBP, launchFlags)
    , diaNotifyRegistered_(false)
{
}

Err iPediaApplication::initialize()
{
    Err error=Application::initialize();
    if (!error)
    {
        if (diaSupport_.available() && isNotifyManager()) 
        {
            error=registerNotify(diaSupport_.notifyType());
            if (!error)
                diaNotifyRegistered_=true;
        }
    }
    if (!error)
    {
        NetLibrary* netLib=0;
        error=getNetLib(netLib);
    }
    return error;
}

iPediaApplication::~iPediaApplication()
{
    if (diaNotifyRegistered_) 
        unregisterNotify(diaSupport_.notifyType());
}


static const UInt32 iPediaRequiredRomVersion=sysMakeROMVersion(3,0,0,sysROMStageDevelopment,0);

Err iPediaApplication::normalLaunch()
{
    Err error=checkRomVersion(iPediaRequiredRomVersion, romIncompatibleAlert);
    if (!error)
    {
        gotoForm(mainForm);
        runEventLoop();
    }
    return error;        
}

Err iPediaApplication::handleSystemNotify(SysNotifyParamType& notify)
{
    const ArsLexis::DIA_Support& dia=getDIASupport();
    if (dia.available() && dia.notifyType()==notify.notifyType)
        dia.handleNotify();
    return errNone;
}

Err iPediaApplication::getNetLib(NetLibrary*& netLib)
{
    Err error=errNone;
    if (!netLib_.get())
    {
        NetLibPtr tmp(new NetLibrary);
        if (tmp.get())
        {
            UInt16 ifError=0;
            error=tmp->initialize(ifError);
            if (!error)
            {
                assert(!ifError);
                netLib_=tmp;
            }
        }
        else
            error=memErrNotEnoughSpace;
    }
    if (!error)
        netLib=netLib_.get();
    return error;
}
