#include "Application.hpp"
#include "Form.hpp"

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
    
    Err Application::setInstance(UInt32 creatorId, Application* app)
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
    
    Application* Application::getInstance(UInt32 creatorId)
    {
        Application* app=0;
        assert(sizeof(app)<=sizeof(UInt32));
        Err error=FtrGet(creatorId, featureInstancePointer, reinterpret_cast<UInt32*>(&app));
        return app;
    }
    
    Application::Application(UInt32 creatorId):
        eventTimeout_(evtWaitForever),
        formEventHandlerThunk_(Form::routeEventToForm),
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
        assert(forms_.empty());
    }

#define kPalmOS20Version sysMakeROMVersion(2,0,0,sysROMStageDevelopment,0)
    
    Err Application::checkRomVersion(UInt32 requiredVersion, UInt16 launchFlags, UInt16 alertId)
    {
        Err error=errNone;
        if (romVersion() < requiredVersion)
        {
            if ((launchFlags & 
                (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)) ==
                (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp))
            {
                if (frmInvalidObjectId!=alertId)
                    FrmAlert (alertId);
                if (romVersion() < kPalmOS20Version)
                    AppLaunchWithCommand(sysFileCDefaultApp, sysAppLaunchCmdNormalLaunch, NULL);
            }
            error=sysErrRomIncompatible;
        }
        return error;
    }
    
    struct FormComparator
    {
        UInt16 id;
        FormComparator(UInt16 anId):
            id(anId)
        {}
        
        bool operator ()(const Form* form) const
        {
            return id==form->id();
        }
        
    };
    
    Form* Application::getOpenForm(UInt16 id) const
    {
        Form* result=0;
        Forms_t::const_iterator it=std::find_if(forms_.begin(), forms_.end(), FormComparator(id));
        if (it!=forms_.end())
            result=*it;
        return result;
    }
    
    Form* Application::createForm(UInt16 formId)
    {
        return new Form(*this, formId); // Form constructor calls FrmInitForm(), and sets event handler.
    }
    
    Err Application::initializeForm(Form& form)
    {
        return form.initialize();
    }
    
    void Application::loadForm(UInt16 formId)
    {
        Err error=errNone;
        // No, that isn't a leak. Form will delete itself when it receives frmCloseEvent or user dismisses it (popup).
        Form* form=createForm(formId);
        if (form)
        {
            error=initializeForm(*form);
            if (!error)
                form->activate();
            else 
                delete form;
        }            
        else
            error=memErrNotEnoughSpace;
    }
    
    Boolean Application::handleApplicationEvent(EventType& event)
    {
        Boolean handled=false;
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

    Err Application::handleLaunchCode(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
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
   
}