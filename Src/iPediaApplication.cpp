
#include "iPediaApplication.hpp"
#include <SysUtils.hpp>
#include "MainForm.hpp"
#include "RegistrationForm.hpp"
#include "SearchResultsForm.hpp"
#include "LookupManager.hpp"

#include <PrefsStore.hpp>

IMPLEMENT_APPLICATION_INSTANCE(appFileCreator)

using namespace ArsLexis;

iPediaApplication::iPediaApplication():
    log_("root"),
    diaNotifyRegistered_(false),
    ticksPerSecond_(SysTicksPerSecond()),
    lookupManager_(0),
    server_(serverLocalhost),
    stressMode_(false),
    hasHighDensityFeatures_(false)
{
#ifdef INTERNAL_BUILD
    log_.addSink(new MemoLogSink(), log_.logError);
#ifndef NDEBUG    
    log_.addSink(new HostFileLogSink("\\var\\log\\iPedia.log"), log_.logEverything);
    log_.addSink(new DebuggerLogSink(), log_.logWarning);
#endif    
#endif

    UInt32 version=0;
    Err error=FtrGet(sysFtrCreator, sysFtrNumWinVersion, &version);
    if (errNone==error && version>=4)
        hasHighDensityFeatures_=true;
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
        if (diaSupport_ && isNotifyManager()) 
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

    logAllocation_=false;        
}


Err iPediaApplication::normalLaunch()
{
    loadPreferences();
    gotoForm(mainForm);
    runEventLoop();
    savePreferences();
    return errNone;
}

Err iPediaApplication::handleSystemNotify(SysNotifyParamType& notify)
{
    const ArsLexis::DIA_Support& dia=getDIASupport();
    if (dia && dia.notifyType()==notify.notifyType)
        dia.handleNotify();
    return errNone;
}

LookupManager* iPediaApplication::getLookupManager(bool create)
{
    Err error=errNone;
    if (!lookupManager_ && create)
        lookupManager_=new LookupManager(history_, &hyperlinkHandler_);
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
            log().debug()<<"Alert: "<<data.alertId;
    }
    if (lookupManager_ && appLookupEventFirst<=event.eType && appLookupEventLast>=event.eType)
        lookupManager_->handleLookupEvent(event);
    else
        handled=Application::handleApplicationEvent(event);
    return handled;
}

namespace {

    enum PreferenceId 
    {
        cookiePrefId,
        serialNumberPrefId,
        serialNumberRegFlagPrefId,
        lookupHistoryFirstPrefId,
        renderingPrefsFirstPrefId=lookupHistoryFirstPrefId+LookupHistory::reservedPrefIdCount,
        
        next=renderingPrefsFirstPrefId+RenderingPreferences::reservedPrefIdCount
    };

    // These globals will be removed by dead code elimination.
    ArsLexis::StaticAssert<(sizeof(uint_t) == sizeof(UInt16))> uint_t_the_same_size_as_UInt16;
    ArsLexis::StaticAssert<(sizeof(bool) == sizeof(Boolean))> bool_the_same_size_as_Boolean;
    
}

void iPediaApplication::loadPreferences()
{
    Preferences prefs;
    // PrefsStoreXXXX seem to be rather heavyweight objects (writer is >480kB), so it might be a good idea not to allocate them on stack.
    std::auto_ptr<PrefsStoreReader> reader(new PrefsStoreReader(appPrefDatabase, appFileCreator, sysFileTPreferences));

    Err error=errNone;
    const char* text;
    if (errNone!=(error=reader->ErrGetStr(cookiePrefId, &text))) 
        goto OnError;
    prefs.cookie=text;
    if (errNone!=(error=reader->ErrGetStr(serialNumberPrefId, &text))) 
        goto OnError;
    prefs.serialNumber=text;
    if (errNone!=(error=reader->ErrGetBool(serialNumberRegFlagPrefId, safe_reinterpret_cast<Boolean*>(&prefs.serialNumberRegistered))))
        goto OnError;
    if (errNone!=(error=prefs.renderingPreferences.serializeIn(*reader, renderingPrefsFirstPrefId)))
        goto OnError;
    preferences_=prefs;    
    if (errNone!=(error=history_.serializeIn(*reader, lookupHistoryFirstPrefId)))
        goto OnError;
    return;
            
OnError:
    return;        
}

void iPediaApplication::savePreferences()
{
    std::auto_ptr<PrefsStoreWriter> writer(new PrefsStoreWriter(appPrefDatabase, appFileCreator, sysFileTPreferences));
    Err error=errNone;
    if (errNone!=(error=writer->ErrSetStr(cookiePrefId, preferences_.cookie.c_str())))
        goto OnError;
    if (errNone!=(error=writer->ErrSetStr(serialNumberPrefId, preferences_.serialNumber.c_str())))
        goto OnError;
    if (errNone!=(error=writer->ErrSetBool(serialNumberRegFlagPrefId, preferences_.serialNumberRegistered)))
        goto OnError;
    if (errNone!=(error=preferences_.renderingPreferences.serializeOut(*writer, renderingPrefsFirstPrefId)))
        goto OnError;
    if (errNone!=(error=history_.serializeOut(*writer, lookupHistoryFirstPrefId)))
        goto OnError;
    if (errNone!=(error=writer->ErrSavePreferences()))
        goto OnError;
    return;        
OnError:
    //! @todo Diplay alert that saving failed?
    return;
}

