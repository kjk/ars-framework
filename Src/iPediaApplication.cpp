
#include "iPediaApplication.hpp"
#include "SysUtils.hpp"
#include "MainForm.hpp"
#include "SocketAddress.hpp"


IMPLEMENT_APPLICATION_INSTANCE(appFileCreator)

using namespace ArsLexis;

iPediaApplication::iPediaApplication():
    diaNotifyRegistered_(false),
    ticksPerSecond_(SysTicksPerSecond())
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


static const UInt32 iPediaRequiredRomVersion=sysMakeROMVersion(3,5,0,sysROMStageDevelopment,0);

Err iPediaApplication::normalLaunch()
{
    setEventTimeout(0);
    NetLibrary* netLib;
    getNetLib(netLib);
    if (netLib)
    {
        SocketConnection* connection=new SocketConnection(*connectionManager_);
        Err error=connection->open(INetSocketAddress(0x7f000001, 80));
//        Err error=connection->open(INetSocketAddress(0x12345678, 80));
        if (!error || netErrWouldBlock==error)
        {
            connection->registerEvent(SocketSelector::eventWrite);
//            connection->registerEvent(SocketSelector::eventException);
        }
        else
            delete connection;
    }

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
                connectionManager_=ConnectionManagerPtr(new SocketConnectionManager(*netLib_));
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
    if (connectionManager_.get() && connectionManager_->active())
    {
        setEventTimeout(0);
        Application::waitForEvent(event);
        if (nilEvent==event.eType)
            connectionManager_->runUntilEvent(ticksPerSecond_/20);
    }
    else
    {
        setEventTimeout(evtWaitForever);
        Application::waitForEvent(event);
    }        
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
