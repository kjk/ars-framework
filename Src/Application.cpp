#include <Application.hpp>
#include <Form.hpp>
#include <FormGadget.hpp>
#include <DeviceInfo.hpp>
#include <ExtendedEvent.hpp>
#include <PalmSysUtils.hpp>

// Explicit instantiation of ArsLexis::String so that we could be sure that all its functions will be in 1st segment and app won't crash on find etc.
template class std::basic_string<ArsLexis::char_t>;

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
    forms_.insert(forms_.begin(), &form);
}

void Application::unregisterForm(Form& form)
{
    for (size_t i = 0; i < forms_.size(); ++i)
        if (&form == forms_[i])
        {
            forms_.erase(forms_.begin() + i);
            break;
        }
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
    runningOnTreo600_(isTreo600()),
    ticksPerSecond_(SysTicksPerSecond())
{
    Err error=SysCurAppDatabase(&cardNo_, &databaseId_);
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
    
    size_t i, size = forms_.size();
    for (i = 0; i < size; ++i)
        delete forms_[i];
    size = enqueuedForms_.size();
    for (i = 0; i < size; ++i)
        delete enqueuedForms_[i];
}

#define kPalmOS20Version sysMakeROMVersion(2,0,0,sysROMStageDevelopment,0)

Err Application::checkRomVersion(UInt32 requiredVersion, UInt16 launchFlags, UInt16 alertId)
{
    Err error = errNone;
    UInt32 romVer = romVersion();
    if (romVer < requiredVersion)
    {
        if ((launchFlags & 
            (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)) ==
            (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp))
        {
            if (frmInvalidObjectId!=alertId)
                alert(alertId);
            if (romVer < kPalmOS20Version)
                AppLaunchWithCommand(sysFileCDefaultApp, sysAppLaunchCmdNormalLaunch, NULL);
        }
        error=sysErrRomIncompatible;
    }
    return error;
}

Form* Application::getOpenForm(UInt16 id) const
{
    for (size_t i = 0; i < forms_.size(); ++i)
        if (forms_[i]->id() == id)
            return forms_[i];
    return NULL;
}

Form* Application::getOpenForm(WinHandle winHandle) const
{
    for (size_t i = 0; i < forms_.size(); ++i)
        if (forms_[i]->windowHandle() == winHandle)
            return forms_[i];
    return NULL;
}

Form* Application::getLastForm() const
{
    if (!forms_.empty())
        return forms_.front();
    return NULL;
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
        enqueuedForms_.erase(enqueuedForms_.begin());
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
        handled = handleExtendedEvent(event);

    return handled;
}

void Application::runEventLoop()
{
    EventType event;
    while (true)
    {
        Err error;
        waitForEvent(event);
        if (appStopEvent == event.eType)
            break;

        if (!SysHandleEvent(&event))
        {
            if (!MenuHandleEvent(0, &event, &error))
            {
                if (!handleApplicationEvent(event))
                {
                    FrmDispatchEvent(&event);
                }
            }
        }

        if (extEvent == event.eType) 
        {
            ExtEventFree(event);
        }
   };
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

bool Application::handleExtendedEvent(EventType& event) 
{
    return false;
}

