
#include "iPediaApplication.hpp"
#include "SysUtils.hpp"
#include "MainForm.hpp"
#include "NetLibrary.hpp"

IMPLEMENT_APPLICATION_INSTANCE(appFileCreator)

using namespace ArsLexis;

iPediaApplication::iPediaApplication():
    diaNotifyRegistered_(false),
    netLib_(0),
    connectionManager_(0),
    ticksPerSecond_(SysTicksPerSecond()),
    resolver_(0)
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
    
    detectViewer();
       
    return error;
}

void iPediaApplication::detectViewer()
{
    DmSearchStateType searchState;
    UInt16 cardNo=0;
    LocalID dbID=0;
    Err error = DmGetNextDatabaseByTypeCreator(true, &searchState, sysFileTApplication, sysFileCClipper, true, &cardNo, &dbID);
    if (!error)
        goto NoError;
    error = DmGetNextDatabaseByTypeCreator(true, &searchState, sysFileTApplication, 'NF3P', true, &cardNo, &dbID);
    if (!error)
        goto NoError;
    error = DmGetNextDatabaseByTypeCreator(true, &searchState, sysFileTApplication, 'NOVR', true, &cardNo, &dbID);
    if (!error)
        goto NoError;
    error = DmGetNextDatabaseByTypeCreator(true, &searchState, sysFileTApplication, 'NF3T', true, &cardNo, &dbID);
    if (!error)
        goto NoError;
    error = DmGetNextDatabaseByTypeCreator(true, &searchState, sysFileTApplication, 'BLZ1', true, &cardNo, &dbID);
    if (!error)
        goto NoError;
    error = DmGetNextDatabaseByTypeCreator(true, &searchState, sysFileTApplication, 'BLZ5', true, &cardNo, &dbID);
    if (!error)
        goto NoError;
    error = DmGetNextDatabaseByTypeCreator(true, &searchState, sysFileTApplication, 'PScp', true, &cardNo, &dbID);
    if (!error)
        goto NoError;
    return;
NoError:
    assert(dbID!=0);
    hyperlinkHandler_.setViewerLocation(cardNo, dbID);
}

iPediaApplication::~iPediaApplication()
{
    if (diaNotifyRegistered_) 
        unregisterNotify(diaSupport_.notifyType());
    
    // Hard to believe, but seems that destructors are in some way accessed even if objects==0. 
    // This causes bus error in non-normal launch (SocketConnectionManager is in 2nd segment).
    // That's why I have to use these ifs below...
    if (connectionManager_)
        delete connectionManager_;
    if (netLib_)        
        delete netLib_;
    if (resolver_)
        delete resolver_;        
}


static const UInt32 iPediaRequiredRomVersion=sysMakeROMVersion(3,5,0,sysROMStageDevelopment,0);

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

Err iPediaApplication::getNetLibrary(NetLibrary*& netLib)
{
    Err error=errNone;
    if (!netLib_)
    {
        NetLibrary* tmp=new NetLibrary;
        UInt16 ifError=0;
        error=tmp->initialize(ifError);
        if (!error)
        {
            assert(!ifError);
            netLib_=tmp;
            connectionManager_=new SocketConnectionManager(*netLib_);
            resolver_=new Resolver(*netLib_);
        }
    }
    if (!error)
        netLib=netLib_;
    return error;
}

void iPediaApplication::waitForEvent(EventType& event)
{
    if (connectionManager_ && connectionManager_->active())
    {
        setEventTimeout(0);
        Application::waitForEvent(event);
        if (nilEvent==event.eType)
            connectionManager_->manageConnectionEvents(ticksPerSecond_/20);
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
        diaSupport_.configureForm(form, 160, pinMaxConstraintSize, pinMaxConstraintSize, 160, pinMaxConstraintSize, pinMaxConstraintSize);
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

Err iPediaApplication::getConnectionManager(SocketConnectionManager*& manager)
{
    NetLibrary* netLib=0;
    Err error=getNetLibrary(netLib);
    if (!error)
        assert(connectionManager_!=0);
    manager=connectionManager_;
    return error;
}

Err iPediaApplication::getResolver(Resolver*& resolver)
{
    NetLibrary* netLib=0;
    Err error=getNetLibrary(netLib);
    if (!error)
        assert(resolver_!=0);
    resolver=resolver_;
    return error;
}
