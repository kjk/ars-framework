#include "Application.hpp"
#include "Form.hpp"

namespace ArsLexis
{
    
    void Application::registerForm(UInt16 id, Form* form)
    {
        assert(form!=0);
#ifndef NDEBUG
        Forms_t::iterator it=forms_.find(id);
        assert(forms_.end()==it);
#endif        
        forms_[id]=form;
    }
    
    void Application::unregisterForm(UInt16 id)
    {
#ifdef NDEBUG
        forms_.erase(id);
#else
        Forms_t::iterator it=forms_.find(id);
        assert(it!=forms_.end());
        forms_.erase(it);
#endif        
    }
    
    void Application::setInstance(UInt32 creatorId, Application* app) throw(std::bad_alloc)
    {
        assert(app!=0);
        Err error=errNone;
#ifndef NDEBUG        
        UInt32 value=0;
        assert(ftrErrNoSuchFeature==FtrGet(creatorId, featureInstancePointer, &value));
#endif        
        assert(sizeof(app)<=sizeof(UInt32));
        error=FtrSet(creatorId, featureInstancePointer, reinterpret_cast<UInt32>(app));
        if (error)
        {
            // According to PalmOS reference memErrNotEnoughSpace is the only possible error code here.
            assert(memErrNotEnoughSpace==error);
            throw std::bad_alloc();
        }
    }
    
    Application& Application::getInstance(UInt32 creatorId) throw()
    {
        Application* app=0;
        assert(sizeof(app)<=sizeof(UInt32));
        Err error=FtrGet(creatorId, featureInstancePointer, reinterpret_cast<UInt32*>(&app));
        assert(!error); // Out application logic will ensure that both this and the following assertions should not fail.
        assert(app!=0);
        return *app;
    }
    
    Application::Application(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags, UInt32 creatorId) throw(std::bad_alloc, PalmOSError):
        eventTimeout_(evtWaitForever),
        launchCommand_(cmd),
        launchParameterBlock_(cmdPBP),
        launchFlags_(launchFlags),
        formEventHandlerThunk_(Form::routeEventToForm),
        cardNo_(0),
        databaseId_(0),
        creatorId_(creatorId)
    {
        Err error=SysCurAppDatabase(&cardNo_, &databaseId_);
        assert(!error);
        setInstance(creatorId_, this);
    }
    
    Application::~Application() throw() 
    {
        FrmCloseAllForms();
        Err error=FtrUnregister(creatorId_, featureInstancePointer);
        assert(!error);
    }

    static const UInt32 kPalmOS20Version=sysMakeROMVersion(2,0,0,sysROMStageDevelopment,0);
    
    void Application::checkRomVersion(UInt32 requiredVersion, UInt16 alertId) throw(PalmOSError)
    {
        UInt32 romVersion=0;
        Err error=FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
        if (error)
            throw PalmOSError(error);
        if (romVersion < requiredVersion)
        {
            if ((launchFlags() & 
                (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)) ==
                (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp))
            {
                if (frmInvalidObjectId!=alertId)
                    FrmAlert (alertId);
                if (romVersion < kPalmOS20Version)
                    AppLaunchWithCommand(sysFileCDefaultApp, sysAppLaunchCmdNormalLaunch, NULL);
            }
            throw PalmOSError(sysErrRomIncompatible);
        }
    }
    
    Form* Application::getOpenForm(UInt16 id) const throw()
    {
        Form* result=0;
        Forms_t::const_iterator it=forms_.find(id);
        if (it!=forms_.end())
            result=it->second;
        return result;
    }
    
    void Application::loadForm(UInt16 formId)
    {
        // No, that isn't a leak. Form will delete itself when it receives frmCloseEvent or user dismisses it (popup).
        Form* form=new Form(*this, formId); // Form constructor calls FrmInitForm(), and sets event handler.
        form->activate();
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
            EvtGetEvent(&event, eventTimeout_);
            if (appStopEvent!=event.eType)
            {
                if (!SysHandleEvent(&event))
                    if (!MenuHandleEvent(0, &event, &error))
                        if (!handleApplicationEvent(event))
                            FrmDispatchEvent(&event);
            }
        } while (appStopEvent!=event.eType);
    }
    
    void Application::registerNotify(UInt32 notifyType, Int8 priority, void* userData) throw(PalmOSError)
    {
        Err error=SysNotifyRegister(cardNumber(), databaseId(), notifyType, NULL, priority, userData);
        if (error)
            throw PalmOSError(error);
    }
    
    void Application::unregisterNotify(UInt32 notifyType, Int8 priority) throw(PalmOSError)
    {
        Err error=SysNotifyUnregister(cardNumber(), databaseId(), notifyType, priority);
        if (error)
            throw PalmOSError(error);
    }

}