#include <Application.hpp>
#include <Form.hpp>
#include <memory>
#include <FormGadget.hpp>
#include <DeviceInfo.hpp>
#include <ExtendedEvent.hpp>

#ifdef ARSLEXIS_USE_MEM_GLUE
# include <MemGlue.h>
# ifdef malloc
#  undef malloc
# endif
# define malloc MemGluePtrNew
#endif

// Explicit instantiation of ArsLexis::String so that we could be sure that all its functions will be in 1st segment and app won't crash on find etc.
template class std::basic_string<ArsLexis::char_t>;

namespace ArsLexis
{

    Boolean FormGadget::gadgetHandler(FormGadgetTypeInCallback* g, UInt16 cmd, void* param)
    {
        if (formGadgetDeleteCmd==cmd)
            return false;
        FormGadget* gadget=const_cast<FormGadget*>(static_cast<const FormGadget*>(g->data));
        assert(0!=gadget);
        assert(gadget->id()==g->id);
        gadget->usable_=g->attr.usable;
        if (formGadgetDrawCmd==cmd && gadget->usable_ && !g->attr.visible)
            g->attr.visible=true;
        gadget->visible_=g->attr.visible;
        if (formGadgetHandleEventCmd==cmd)
        {
            assert(0!=param);
            EventType* event=static_cast<EventType*>(param);
            if (frmGadgetEnterEvent==event->eType)
                gadget->form()->trackingGadget_=gadget;
        }
        return gadget->handleGadgetCommand(cmd, param);
    }
    
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

    namespace {
        static const StaticAssert<sizeof(Application*)<=sizeof(UInt32)> may_store_app_pointer_as_feature = {};
    }
    
    Err Application::setInstance(UInt32 creatorId, Application* app) throw()
    {
        assert(app!=0);
        Err error;
#ifndef NDEBUG        
        UInt32 value=0;
        assert(ftrErrNoSuchFeature==FtrGet(creatorId, featureInstancePointer, &value));
#endif        
        error=FtrSet(creatorId, featureInstancePointer, reinterpret_cast<UInt32>(app));
        return error;
    }
    
    Application* Application::getInstance(UInt32 creatorId) throw()
    {
        Application* app;
        Err error=FtrGet(creatorId, featureInstancePointer, reinterpret_cast<UInt32*>(&app));
        return app;
    }
    
#ifndef NDEBUG
#pragma inline_depth 0
#endif

    Application::Application():
        eventTimeout_(evtWaitForever),
#ifdef __MWERKS__        
        formEventHandlerThunk_(Form::routeEventToForm),
        gadgetHandlerThunk_(FormGadget::gadgetHandler),
#endif // __MWERKS__        
        cardNo_(0),
        databaseId_(0),
        creatorId_(creator()),
        romVersion_(0),
        runningOnTreo600_(isTreo600()),
        underSimulator_(ArsLexis::underSimulator()),
        ticksPerSecond_(SysTicksPerSecond())
    {
        Err error=SysCurAppDatabase(&cardNo_, &databaseId_);
        assert(!error);
        error=FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion_);
        assert(!error);        
    }

#ifndef NDEBUG
#pragma inline_depth 2
#endif
    
    Err Application::initialize()
    {
        return setInstance(creatorId_, this);
    }
    
    Application::~Application() 
    {
        Err error=FtrUnregister(creatorId_, featureInstancePointer);
        assert(!error);
        std::for_each(forms_.begin(), forms_.end(), ObjectDeleter<Form>());   
        std::for_each(enqueuedForms_.begin(), enqueuedForms_.end(), ObjectDeleter<Form>());   
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
                    alert(alertId);
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
    
    Form* Application::getLastForm() const
    {
        Form* f = NULL;
        if (!forms_.empty())
            f = forms_.front();
        return f;
    }

    
    Form* Application::createForm(UInt16 formId)
    {
        return new Form(*this, formId); // Form constructor calls FrmInitForm(), and sets event handler.
    }
    
    void Application::loadForm(UInt16 formId)
    {
        Err error;
        Form* form;
        if (!enqueuedForms_.empty() && enqueuedForms_.front()->id() == formId)
        {
            form = enqueuedForms_.front();
            enqueuedForms_.pop_front();
        }
        else
            form = createForm(formId);
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
        UInt32 time = TimGetTicks();
        bool handled=false;
        if (frmLoadEvent == event.eType)
        {
            loadForm(event.data.frmLoad.formID);
            handled=true;
        }
        else if (extEvent == event.eType) 
        {
            ExtendedEventData& data = reinterpret_cast<ExtendedEventData&>(event.data);
            handled = handleExtendedEvent(data.eventType, data.properties);
        }
        return handled;
    }
    
    void Application::runEventLoop()
    {
        EventType event;
        do
        {
            Err error;
            waitForEvent(event);
            if (appStopEvent!=event.eType)
            {
                if (!SysHandleEvent(&event))
                    if (!MenuHandleEvent(0, &event, &error))
                        if (!handleApplicationEvent(event))
                            FrmDispatchEvent(&event);
                if (extEvent == event.eType) 
                {
                    ExtendedEventData& data = reinterpret_cast<ExtendedEventData&>(event.data);
                    data.dispose();
                    assert(NULL == data.properties);
                }
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

/*    
    UInt32 Application::creator()
    {
        UInt16 cardNo;
        LocalID localId;
        Err error=SysCurAppDatabase(&cardNo, &localId);
        assert(errNone==error);
        UInt32 creatorId;
        error=DmDatabaseInfo(cardNo, localId, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &creatorId);
        assert(errNone==error);
        return creatorId;
    }
*/

    Application& Application::instance()
    {
        UInt32 crId;
        Application* app=getInstance(crId=creator());
        assert(app!=0); \
        return  *app;\
    }
    
    void Application::startForm(Form* form, bool popup)
    {
        enqueuedForms_.push_back(form);
        if (popup)
            popupForm(form->id());
        else
            gotoForm(form->id());                
    }
    
    bool Application::handleExtendedEvent(uint_t, EventProperties*) 
    {
        return false;
    }
    
}

void* ArsLexis::allocate(size_t size)
{
    void* ptr=0;
    if (size) 
        ptr = malloc(size);
    else
        ptr = malloc(1);
    if (!ptr)
        handleBadAlloc();
    return ptr;
}
