
#include "iPediaApplication.hpp"
#include <SysUtils.hpp>
#include "MainForm.hpp"
#include "RegistrationForm.hpp"
#include "SearchResultsForm.hpp"
#include "LookupManager.hpp"

IMPLEMENT_APPLICATION_INSTANCE(appFileCreator)

using namespace ArsLexis;

iPediaApplication::iPediaApplication():
    log_("root"),
    diaNotifyRegistered_(false),
    ticksPerSecond_(SysTicksPerSecond()),
    lookupManager_(0),
    server_(serverLocalhost),
    stressMode_(false)
{
#ifndef NDEBUG
    log_.addSink(new MemoLogSink(), log_.logError);
    log_.addSink(new HostFileLogSink("\\var\\log\\iPedia.log"), log_.logEverything);
    log_.addSink(new DebuggerLogSink(), log_.logEverything);
#endif
}

inline void iPediaApplication::detectViewer()
{
    UInt16  cardNo;
    LocalID dbID;

    if (fDetectViewer(&cardNo,&dbID))
    {
        assert(dbID!=0);
        hyperlinkHandler_.setViewerLocation(cardNo, dbID);
    }
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

iPediaApplication::~iPediaApplication()
{
    if (diaNotifyRegistered_) 
        unregisterNotify(diaSupport_.notifyType());
    
    if (lookupManager_)
        delete lookupManager_;    
}


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

LookupManager* iPediaApplication::getLookupManager(bool create)
{
    Err error=errNone;
    if (!lookupManager_ && create)
    {
        std::auto_ptr<LookupManager> tmp(new LookupManager());
        error=tmp->initialize();
        if (errNone==error)
            lookupManager_=tmp.release();
        else
            sendDisplayAlertEvent(networkUnavailableAlert);
    }
    return lookupManager_;
}

void iPediaApplication::waitForEvent(EventType& event)
{
    ArsLexis::SocketConnectionManager* manager=0;
    if (lookupManager_)
        manager=&lookupManager_->connectionManager();
    if (manager && manager->active())
    {
        setEventTimeout(0);
        Application::waitForEvent(event);
        if (nilEvent==event.eType)
            manager->manageConnectionEvents(ticksPerSecond_/20);
    }
    else
    {
        setEventTimeout(evtWaitForever);
        Application::waitForEvent(event);
    }        
}

Form* iPediaApplication::createForm(UInt16 formId)
{
    Form* form=0;
    switch (formId)
    {
        case mainForm:
            form=new MainForm(*this);
            break;
            
        case registrationForm:
            form=new RegistrationForm(*this);
            break;
            
        case searchResultsForm:
            form=new SearchResultsForm(*this);
            break;
        
        default:
            assert(false);
    }
    return form;            
}

bool iPediaApplication::handleApplicationEvent(EventType& event)
{
    bool handled=false;
    if (appDisplayAlertEvent==event.eType)
    {
        DisplayAlertEventData& data=reinterpret_cast<DisplayAlertEventData&>(event.data);
        if (!inStressMode())
            FrmAlert(data.alertId);
        else
            log()<<"Alert: "<<data.alertId;
    }
    if (lookupManager_ && appLookupEventFirst<=event.eType && appLookupEventLast>=event.eType)
        lookupManager_->handleLookupEvent(event);
    else
        handled=Application::handleApplicationEvent(event);
    return handled;
}
