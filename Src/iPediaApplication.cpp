
#include "iPediaApplication.hpp"
#include "SysUtils.hpp"
#include "MainForm.hpp"


IMPLEMENT_APPLICATION_INSTANCE(appFileCreator)

using namespace ArsLexis;

iPediaApplication::iPediaApplication():
    diaNotifyRegistered_(false)
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
    gotoForm(mainForm);
    runEventLoop();
    return errNone;
}

Err iPediaApplication::handleSystemNotify(SysNotifyParamType& notify)
{
    const ArsLexis::DIA_Support& dia=getDIASupport();
    if (dia.available() && dia.notifyType()==notify.notifyType)
        dia.handleNotify();
    return errNone;
}

Err iPediaApplication::handleLaunchCode(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
    Err error=checkRomVersion(iPediaRequiredRomVersion, launchFlags, romIncompatibleAlert);
    if (!error)
        error=Application::handleLaunchCode(cmd, cmdPBP, launchFlags);
    return error;
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

void iPediaApplication::waitForEvent(EventType& event)
{
    //! @todo use netLib's select to implement non-blocking network io.
    Application::waitForEvent(event);
}

Err iPediaApplication::initializeForm(Form& form)
{
    Err error=Application::initializeForm(form);
    if (!error && diaSupport_.available())
        diaSupport_.configureForm(form, 160, 160, 225, 160,160, 225);
    return error;
}

Form* iPediaApplication::createForm(UInt16 formId)
{
    Form* form=0;
    switch (formId)
    {
        case mainForm:
            form=new MainForm(*this);
            break;
        
        default:
            assert(false);
    }
    return form;            
}
