#include <Form.hpp>
#include <Application.hpp>
#include <Graphics.hpp>
#include <FormGadget.hpp>
#include <SysUtils.hpp>
#include <68k/Hs.h>
#include <ExtendedEvent.hpp>
#include <Text.hpp>

static void updateForm(int formId, UInt16 updateCode)
{
    EventType event;
    MemSet(&event, sizeof(event), 0);
    event.eType=frmUpdateEvent;
    event.data.frmUpdate.formID=formId;
    event.data.frmUpdate.updateCode=updateCode;
    EvtAddUniqueEventToQueue(&event, 0, false);
}

namespace ArsLexis 
{

    Boolean Form::routeEventToForm(EventType* event)
    {
        Form* form = NULL;
        Application& app = Application::instance();
        eventsEnum eventType = event->eType;
        if ((frmLoadEvent <= eventType && frmTitleSelectEvent >= eventType) && frmSaveEvent != eventType)
        {
            form = app.getOpenForm(event->data.frmLoad.formID);  // All of these events take formID as their first data member.
            if (NULL == form && frmUpdateEvent == eventType && 0 == event->data.frmUpdate.formID)
                form = app.getLastForm();
        }
        else
        {
            switch (eventType)
            {
                case winExitEvent:
                    form = app.getOpenForm(event->data.winExit.exitWindow);
                    break;
                    
                case winEnterEvent:
                    form = app.getOpenForm(event->data.winEnter.enterWindow);
                    break;
                    
                default:
                    form = app.getOpenForm(FrmGetActiveFormID());
            }
            
        }
        Boolean result=false;
        if (form)
        {   
            if (NULL != form->trackingGadget_)
            {
                result = form->trackingGadget_->handleEvent(*event);
                if (penUpEvent == event->eType)
                    form->trackingGadget_ = NULL;
            }
            if (!result)
            {
                if (!form->controlsAttached_)
                    form->attachControls();
                result = form->handleEvent(*event);
            }
            if (frmInvalidObjectId != form->returnToFormId_)
            {
                form->deleteOnClose_ = false;
                form->handleClose();
                FrmReturnToForm(form->returnToFormId_);
                // force update of the form we'rereturning to
                // if we don't we might get some garbage on the form
                // (e.g. gadgets from previous form)
                updateForm(form->returnToFormId_, frmRedrawUpdateCode);
                form->returnToFormId_ = frmInvalidObjectId;
                form->form_ = NULL;
                form->id_ = frmInvalidObjectId;
                assert(form->deleteAfterEvent_);
            }
            if (form->deleteAfterEvent_)
            {
//                result=true; // If we don't return true after Frm
                delete form;
            }                
        }
        return result;
    }
    
    Form::Form(Application& app, UInt16 id):
        application_(app),
        id_(id),
        form_(0), 
        title_(NULL),
        deleteOnClose_(true),
        deleteAfterEvent_(false),
        controlsAttached_(false),
        trackingGadget_(0),
        entryFocusControlId_(frmInvalidObjectId),
        focusedControlIndex_(noFocus),
        returnToFormId_(frmInvalidObjectId),
        formOpenReceived_(false),
        winEnterReceived_(false)
    {
        getScreenBounds(screenBoundsBeforeWinExit_);
    }
    
    void Form::attachControls()
    {
        assert(!controlsAttached_);
        controlsAttached_ = true;
    }
    
    Err Form::initialize()
    {
        Err error=errNone;
        form_=FrmInitForm(id());
        if (form_)
        {
#ifdef __MWERKS__        
            FrmSetEventHandler(form_, application_.formEventHandlerThunk_);
#else
            FrmSetEventHandler(form_, routeEventToForm);
#endif // __MWERKS__                        
            application_.registerForm(*this);
        }
        else 
            error=memErrNotEnoughSpace;
        return error;
    }
    
    Form::~Form() 
    {
        if (NULL != title_)
            free(title_);
        if (deleteOnClose_) 
            FrmDeleteForm(form_);
        application_.unregisterForm(*this);
    }
    
    void Form::releaseFocus()
    {
        FrmSetFocus(form_, noFocus);
        if (application().runningOnTreo600())
            HsNavRemoveFocusRing(form_);
        focusedControlIndex_ = frmInvalidObjectId;
    }
    
    static void FocusField(FormType* form, UInt16 index, UInt16 id)
    {
        FieldType* field = static_cast<FieldType*>(FrmGetObjectPtr(form, index));
        assert(NULL != field);
        FrmSetFocus(form, index);
        FldSetSelection(field, 0, FldGetTextLength(field));
        FrmSetNavState(form, kFrmNavStateFlagsObjectFocusMode);
        RectangleType r;
        FrmGetObjectBounds(form, index, &r);
        // BUG: Treo600 draws focus ring on flashing insertion point and after focus is lost it restores what was underneath
        // (in this case part of cursor). That's why I make focus ring 2px wider than field.
//        --r.topLeft.x;
//        ++r.extent.x;
        HsNavDrawFocusRing(form, id, hsNavFocusRingNoExtraInfo, &r, hsNavFocusRingStyleObjectTypeDefault, true);
    }
    
    bool Form::handleObjectFocusChange(const EventType& event)
    {
        FormGadget* gadget;
        bool taking = (event.eType == frmObjectFocusTakeEvent);
        const frmObjectFocusTake& data = reinterpret_cast<const frmObjectFocusTake&>(event.data);
        assert(id_ == data.formID);
        UInt16 index = FrmGetObjectIndex(form_, data.objectID);

        UInt16 lastIndex = focusedControlIndex_;
        if (taking)
            focusedControlIndex_ = index;

        if (frmInvalidObjectId != lastIndex && frmGadgetObj == FrmGetObjectType(form_, lastIndex) && taking && index != lastIndex)
        {
            gadget = static_cast<FormGadget*>(FrmGetGadgetData(form_, lastIndex));
            assert(NULL != gadget);
            gadget->handleFocusChange(FormGadget::focusLosing);
        }
        if (frmInvalidObjectId == index)
            return false;
        FormObjectKind kind = FrmGetObjectType(form_, index);
        if (taking && frmFieldObj == kind && application().runningOnTreo600())
        {
            if (!visible())
                draw();
            FocusField(form_, index, data.objectID);
            return true;
        }
        if (frmGadgetObj != kind)
            return false;
        gadget = static_cast<FormGadget*>(FrmGetGadgetData(form_, index));
        assert(NULL != gadget);
        if (taking)
            gadget->handleFocusChange(FormGadget::focusTaking);
        return false;
    }
    
    bool Form::handleEvent(EventType& event)
    {
        bool handled=false;
        switch (event.eType)
        {
            case frmObjectFocusTakeEvent:
            case frmObjectFocusLostEvent:
                handled = handleObjectFocusChange(event);                
                break;
                
            case winEnterEvent:
                handled=handleWindowEnter(event.data.winEnter);
                break;

            case winExitEvent:
                handled=handleWindowExit(event.data.winExit);
                break;

            case frmOpenEvent:
                handled=handleOpen();
                break;

            case frmCloseEvent:
                handled=handleClose();
                break;

            case frmUpdateEvent:
                handled=handleUpdate(event.data.frmUpdate.updateCode);
                break;

            case menuEvent:
                handled=handleMenuCommand(event.data.menu.itemID);
                break;
                
            case fldEnterEvent:
                focusedControlIndex_ = FrmGetObjectIndex(form_, event.data.fldEnter.fieldID);
                break;
            
            case frmGadgetEnterEvent:
                focusedControlIndex_ = FrmGetObjectIndex(form_, event.data.gadgetEnter.gadgetID);
                break;
            
            case tblEnterEvent:
                focusedControlIndex_ = FrmGetObjectIndex(form_, event.data.tblEnter.tableID);
                break;
            
            case extEvent: 
            {
                char *data = reinterpret_cast<char*>(&(event.data));
                handled = handleExtendedEvent((void*)data);
                break;
            }
        }
        return handled;
    }
    
    UInt16 Form::showModal()
    {
        assert(form_!=0);
        UInt16 result=FrmDoDialog(form_);
        return result;
    }

    bool Form::handleClose() 
    {
        deleteAfterEvent_ = true;  
        return false;
    }
    
    bool Form::handleOpen()
    {
        assert(controlsAttached_);
        formOpenReceived_ = true;
        deleteOnClose_ = false;
        update();
        return true;
    }
    
    void Form::handleFocusOnEntry() 
    {
        if (frmInvalidObjectId != entryFocusControlId_)
        {
            FormObject object(*this, entryFocusControlId_);
            object.focus();
            entryFocusControlId_ = frmInvalidObjectId;
            if (visible())
                object.draw();
        }
    }
    
    bool Form::handleWindowEnter(const struct _WinEnterEventType& data)
    {
        assert(NULL != form_);
        assert(controlsAttached_);
        if (static_cast<const void*>(form_) == data.enterWindow)
        {
            RectangleType newBounds;
            getScreenBounds(newBounds);
            if (newBounds.extent.x != screenBoundsBeforeWinExit_.extent.x || newBounds.extent.y != screenBoundsBeforeWinExit_.extent.y) 
            {
                EventType event;
                MemSet(&event, sizeof(event), 0);
                event.eType=(eventsEnum)winDisplayChangedEvent;
                event.data.winDisplayChanged.newBounds = newBounds;
                EvtAddUniqueEventToQueue(&event, 0, true);
            }
            winEnterReceived_ = true;
        }
        return false;
    }
    
    bool Form::handleUpdate(UInt16 updateCode)
    {
        draw(updateCode);
        return true;
    }

    void Form::returnToForm(UInt16 formId)
    {
        returnToFormId_ = formId;    
    }
    
    void Form::setBounds(const Rectangle& bounds)
    {
        assert(form_!=0);
        RectangleType native=toNative(bounds);
        WinSetBounds(windowHandle(), &native);
    }        


    void Form::setTitle(const char_t* title)
    {
        assert(form_!=0);
        FrmSetTitle(form_, "");
        char_t *newTitle = StringCopy2(title,-1);
        if (NULL == newTitle)
            return;
        title_ = newTitle;
        //! @bug On Sony Clie (OS 4.1) I experience bug described in Reference as corrected in post-OS 3.0... (Previous title is not erased)
        if (visible())
        {
            Graphics graph(windowHandle());
            Rectangle rect(bounds());
            rect.height()=18;
            graph.erase(rect);
        }        

        FrmSetTitle(form_, (char*)title_);            
    }
    
    const char_t *Form::title()
    {
        assert(form_!=0);
        if (NULL == title_)
        {
            title_ = StringCopy2(FrmGetTitle(form_), -1);
        }
        return title_;
    }
    
    bool Form::handleWindowExit(const struct _WinExitEventType& data)
    {
        const FormType* form=*this;
        if (data.exitWindow==static_cast<const void*>(form))
        {
        /*
            if (!application().runningOnTreo600())
            {
                UInt16 index = FrmGetFocus(form);
                if (noFocus != index)
                    entryFocusControlId_ = FrmGetObjectId(form, index);
                releaseFocus();
            }
            else 
                entryFocusControlId_ = frmInvalidObjectId;
        */                
            getScreenBounds(screenBoundsBeforeWinExit_);
        }
        return false;
    }
    
    void Form::update(UInt16 updateCode)
    {
        EventType event;
        MemSet(&event, sizeof(event), 0);
        event.eType=frmUpdateEvent;
        event.data.frmUpdate.formID=id();
        event.data.frmUpdate.updateCode=updateCode;
        EvtAddUniqueEventToQueue(&event, 0, false);
    }

    UInt16 Form::getGraffitiStateIndex() const
    {
        UInt16 count = FrmGetNumberOfObjects(form_);
        for (UInt16 index = 0; index<count; ++index)
            if (frmGraffitiStateObj == FrmGetObjectType(form_, index))
                return index;
        return frmInvalidObjectId;
    }


    void Form::popup()
    {
        application().popupForm(this);
    }
    
    void Form::run()
    {
        application().gotoForm(this);
    }
    
    /*
    bool Form::handleFocusTransfer(EventType& event)
    {
        if (!application().runningOnTreo600())
            return false;
        if (frmInvalidObjectId == focusedControlIndex_)
            return false;
        bool left = false;
        FormObject object(*this);
        object.attachByIndex(focusedControlIndex_);
        if (fiveWayLeftPressed(&event))
        {
            if (frmFieldObj == object.type() && 0 != FldGetInsPtPosition(static_cast<FieldType*>(object.wrappedObject())))
                return false;
            left = true;
        }
        else if (fiveWayUpPressed(&event))
            left = true;
        else if (fiveWayRightPressed(&event))
        {
            const FieldType* field = static_cast<const FieldType*>(object.wrappedObject());
            if (frmFieldObj == object.type() && FldGetTextLength(field) != FldGetInsPtPosition(field))
                return false;
        }
        else if (!fiveWayDownPressed(&event))
            return false;
        UInt16 index;
        if (left)
        {
            if (0 == focusedControlIndex_)
                return false;
            index = focusedControlIndex_;
            while (true)
            {
                object.attachByIndex(--index);
                if (object.focusable())
                    break;
                if (0 == index)
                    return false;
            }
        }
        else
        {
            const UInt16 objCount = FrmGetNumberOfObjects(form_);
            if (objCount - 1 == focusedControlIndex_)
                return false;
            index = focusedControlIndex_;
            while (true)
            {
                object.attachByIndex(++index);
                if (object.focusable())
                    break;
                if (objCount - 1 == index)
                    return false;
            }
        }
        assert(object.valid());
        assert(object.focusable());
        object.focus();
        return true;
    }
*/

    void Form::draw(UInt16 updateCode)
    {
        FrmDrawForm(form_);
        if (formOpenReceived_ && winEnterReceived_)
            handleFocusOnEntry();
    }

    bool Form::handleExtendedEvent(void *eventData) 
    {
        return false;
    }
    
    
    UInt16 Form::createGadget(UInt16 id, const Rectangle& rect)
    {
        FormGadgetType* g = FrmNewGadget(&form_, id, rect.x(), rect.y(), rect.width(), rect.height());
        if (NULL == g)
            return frmInvalidObjectId;
        UInt16 index = FrmGetObjectIndexFromPtr(form_, g);
        assert(frmInvalidObjectId != index);
        return index;
    }
    
}

