
#include "iPediaApplication.hpp"
#include "ipedia_Rsc.h"

IMPLEMENT_APPLICATION_INSTANCE(appFileCreator)

iPediaApplication::iPediaApplication(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags):
    Application(cmd, cmdPBP, launchFlags),
    diaNotifyRegistered_(false)
{
    if (diaSupport_.available())    {
        UInt32 value=0;
        Err error=FtrGet(sysFtrCreator,sysFtrNumNotifyMgrVersion, &value);
        if (!error && value) try 
        {
            registerNotify(diaSupport_.notifyType());
            diaNotifyRegistered_=true;
        }
        catch (const PalmOSError&)
        {
        }
    }
}

iPediaApplication::~iPediaApplication() throw()
{
    if (diaNotifyRegistered_) try
    {
        unregisterNotify(diaSupport_.notifyType());
    }
    catch (const PalmOSError&)
    {
    }
}


static const UInt32 iPediaRequiredRomVersion=sysMakeROMVersion(3,0,0,sysROMStageDevelopment,0);

void iPediaApplication::run()
{
    if (sysAppLaunchCmdNormalLaunch==launchCommand())
    {
        checkRomVersion(iPediaRequiredRomVersion, RomIncompatibleAlert);
        gotoForm(MainForm);
        runEventLoop();
    }
    else if (sysAppLaunchCmdNotify==launchCommand())
    {
        SysNotifyParamType* notify=static_cast<SysNotifyParamType*>(launchParameterBlock());
        handleSystemNotify(*notify);
    }
}

void iPediaApplication::handleSystemNotify(SysNotifyParamType& notify)
{
    const ArsLexis::DIA_Support& dia=getDIASupport();
    if (dia.available() && dia.notifyType()==notify.notifyType)
        dia.handleNotify();
}