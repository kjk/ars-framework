#include <Application.hpp>
#include <Form.hpp>

#include <memory>

// Explicit instantiation of ArsLexis::String so that we could be sure that all its functions will be in 1st segment and app won't crash on find etc.
template class std::basic_string<ArsLexis::char_t>;
//template class std::char_traits<ArsLexis::char_t>;
//template class std::list<ArsLexis::Form*>;

namespace ArsLexis
{
    
    void Application::registerForm(Form& form)
    {
#ifndef NDEBUG
        Forms_t::iterator it=std::find(forms_.begin(), forms_.end(), &form);
        assert(forms_.end()==it);
#endif        
        forms_.push_front(&form);
    }
    
    void Application::unregisterForm(Form& form)
    {
        Forms_t::iterator it=std::find(forms_.begin(), forms_.end(), &form);
        assert(it!=forms_.end());
        forms_.erase(it);
    }
    
    Err Application::setInstance(UInt32 creatorId, Application* app) throw()
    {
        assert(app!=0);
        Err error=errNone;
#ifndef NDEBUG        
        UInt32 value=0;
        assert(ftrErrNoSuchFeature==FtrGet(creatorId, featureInstancePointer, &value));
#endif        
        assert(sizeof(app)<=sizeof(UInt32));
        error=FtrSet(creatorId, featureInstancePointer, reinterpret_cast<UInt32>(app));
        return error;
    }
    
    Application* Application::getInstance(UInt32 creatorId) throw()
    {
        Application* app=0;
        assert(sizeof(app)<=sizeof(UInt32));
        Err error=FtrGet(creatorId, featureInstancePointer, reinterpret_cast<UInt32*>(&app));
        return app;
    }
    
    Application::Application(UInt32 creatorId):
        eventTimeout_(evtWaitForever),
#ifdef __MWERKS__        
        formEventHandlerThunk_(Form::routeEventToForm),
#endif // __MWERKS__        
        cardNo_(0),
        databaseId_(0),
        creatorId_(creatorId),
        romVersion_(0)
    {
        Err error=SysCurAppDatabase(&cardNo_, &databaseId_);
        assert(!error);
        error=FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion_);
        assert(!error);        
    }
    
    Err Application::initialize()
    {
        return setInstance(creatorId_, this);
    }
    
    Application::~Application() 
    {
        Err error=FtrUnregister(creatorId_, featureInstancePointer);
        assert(!error);
        std::for_each(forms_.begin(), forms_.end(), ObjectDeleter<Form>());
    }

#define kPalmOS20Version sysMakeROMVersion(2,0,0,sysROMStageDevelopment,0)
    
    Err Application::checkRomVersion(UInt32 requiredVersion, UInt16 launchFlags, UInt16 alertId)
    {
        UInt32 romVersion;
        Err error=FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
        if (romVersion < requiredVersion)
        {
            if ((launchFlags & 
                (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)) ==
                (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp))
            {
                if (frmInvalidObjectId!=alertId)
                    FrmAlert (alertId);
                if (romVersion < kPalmOS20Version)
                    AppLaunchWithCommand(sysFileCDefaultApp, sysAppLaunchCmdNormalLaunch, NULL);
            }
            error=sysErrRomIncompatible;
        }
        return error;
    }
    
    namespace {
    
        struct FormIdComparator
        {
            UInt16 id;
            FormIdComparator(UInt16 anId):
                id(anId)
            {}
            
            bool operator ()(const Form* form) const
            {
                return id==form->id();
            }
            
        };

        struct WinHandleComparator
        {
            WinHandle winHandle;
            WinHandleComparator(WinHandle wh):
                winHandle(wh)
            {}
            
            bool operator()(const Form* form) const
            {
                return form->windowHandle()==winHandle;
            }
            
        };

    }
    
    Form* Application::getOpenForm(UInt16 id) const
    {
        Form* result=0;
        Forms_t::const_iterator it=std::find_if(forms_.begin(), forms_.end(), FormIdComparator(id));
        if (it!=forms_.end())
            result=*it;
        return result;
    }
   
    Form* Application::getOpenForm(WinHandle winHandle) const
    {
        Form* result=0;
        Forms_t::const_iterator it=std::find_if(forms_.begin(), forms_.end(), WinHandleComparator(winHandle));
        if (it!=forms_.end())
            result=*it;
        return result;
    }
    
    
    Form* Application::createForm(UInt16 formId)
    {
        return new Form(*this, formId); // Form constructor calls FrmInitForm(), and sets event handler.
    }
    
    void Application::loadForm(UInt16 formId)
    {
        Err error=errNone;
        Form* form(createForm(formId));
        if (form)
        {
            error=form->initialize();
            if (!error)
                form->activate();
                // No, that isn't a leak. Form will delete itself when it receives frmCloseEvent or user dismisses it (popup).
            else
                delete form;
        }            
    }
    
    bool Application::handleApplicationEvent(EventType& event)
    {
        bool handled=false;
        if (frmLoadEvent==event.eType)
        {
            loadForm(event.data.frmLoad.formID);
            handled=true;
        }
        return handled;
    }
    
    void Application::runEventLoop()
    {
        EventType event;
        do
        {
            Err error=errNone;
            waitForEvent(event);
            if (appStopEvent!=event.eType)
            {
                if (!SysHandleEvent(&event))
                    if (!MenuHandleEvent(0, &event, &error))
                        if (!handleApplicationEvent(event))
                            FrmDispatchEvent(&event);
            }
        } while (appStopEvent!=event.eType);
    }
    
    Err Application::registerNotify(UInt32 notifyType, Int8 priority, void* userData)
    {
        return SysNotifyRegister(cardNumber(), databaseId(), notifyType, NULL, priority, userData);
    }
    
    Err Application::unregisterNotify(UInt32 notifyType, Int8 priority)
    {
        return SysNotifyUnregister(cardNumber(), databaseId(), notifyType, priority);
    }

    Err Application::handleLaunchCode(UInt16 cmd, MemPtr cmdPBP, UInt16)
    {
        Err error=errNone;
        if (sysAppLaunchCmdNormalLaunch==cmd)
        {
            error=normalLaunch();
            FrmCloseAllForms();
        }
        else if (sysAppLaunchCmdNotify==cmd)
        {
            SysNotifyParamType* notify=static_cast<SysNotifyParamType*>(cmdPBP);
            error=handleSystemNotify(*notify);
        }
        return error;
    }
    
    
    void Application::waitForEvent(EventType& event)
    {
        EvtGetEvent(&event, eventTimeout_);
    }
    
    void Application::sendEvent(UInt16 eventId, const void* eventData, UInt16 dataLength) throw()
    {
        EventType event;
        MemSet(&event, sizeof(event), 0);
        event.eType=static_cast<eventsEnum>(eventId);
        if (eventData && dataLength)
        {
            assert(dataLength<=sizeof(event.data));
            MemMove(&event.data, eventData, dataLength);
        }
        EvtAddEventToQueue(&event);
    }
   
}