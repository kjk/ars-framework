#include <Form.hpp>
#include <Application.hpp>
#include <Graphics.hpp>
#include <FormGadget.hpp>
#include <SysUtils.hpp>
#include <68k/Hs.h>

namespace ArsLexis 
{

    Boolean Form::routeEventToForm(EventType* event)
    {
        Form* form=0;
        Application& app=Application::instance();
        eventsEnum eventType=event->eType;
        if ((frmLoadEvent<=eventType && frmTitleSelectEvent>=eventType) && frmSaveEvent!=eventType)
            form=app.getOpenForm(event->data.frmLoad.formID);  // All of these events take formID as their first data member.
        else
        {
            switch (eventType)
            {
                case winExitEvent:
                    form=app.getOpenForm(event->data.winExit.exitWindow);
                    break;
                    
                case winEnterEvent:
                    form=app.getOpenForm(event->data.winEnter.enterWindow);
                    break;
                    
                default:
                    form=app.getOpenForm(FrmGetActiveFormID());
            }
            
        }
        Boolean result=false;
        if (form)
        {   
            if (NULL != form->trackingGadget_)
            {
                if (penMoveEvent == event->eType)
                    result = form->trackingGadget_->handleEvent(*event);
                else if (penUpEvent == event->eType)
                {
                    result = form->trackingGadget_->handleEvent(*event);
                    form->trackingGadget_=0;
                }
            }
            if (!result)
            {
                if (!form->controlsAttached_)
                    form->attachControls();
                result = form->handleEvent(*event);
//                if (!result && keyDownEvent == event->eType && app.runningOnTreo600())
//                    result = form->handleFocusTransfer(*event);
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
        deleteOnClose_(true),
        deleteAfterEvent_(false),
        controlsAttached_(false),
        trackingGadget_(0),
        entryFocusControlId_(frmInvalidObjectId),
        focusedControlIndex_(frmInvalidObjectId)
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
    
    void Form::handleObjectFocusChange(const EventType& event)
    {
        const frmObjectFocusTake& data = reinterpret_cast<const frmObjectFocusTake&>(event.data);
        assert(id_ == data.formID);
        UInt16 index = FrmGetObjectIndex(form_, data.objectID);
        if (frmInvalidObjectId == index)
            return;
        bool hasFocus = (event.eType == frmObjectFocusTakeEvent);
        if (hasFocus)
            focusedControlIndex_ = index;
        FormObjectKind kind = FrmGetObjectType(form_, index);
        if (frmGadgetObj != kind)
            return;
        FormGadget* gadget = static_cast<FormGadget*>(FrmGetGadgetData(form_, index));
        assert(NULL != gadget);
        gadget->handleFocusChange(hasFocus ? FormGadget::focusTaking : FormGadget::focusLosing);
    }
    
    bool Form::handleEvent(EventType& event)
    {
        bool handled=false;
        switch (event.eType)
        {
            case frmObjectFocusTakeEvent:
            case frmObjectFocusLostEvent:
                handleObjectFocusChange(event);                
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
        deleteAfterEvent_=true;  
        return false;
    }
    
    bool Form::handleOpen()
    {
        assert(controlsAttached_);
        deleteOnClose_ = false;
        update();
        return true;
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
            if (frmInvalidObjectId != entryFocusControlId_)
            {
                FormObject object(*this, entryFocusControlId_);
                object.focus();
                update();
            }
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
        deleteOnClose_=false;
        handleClose();        
        FrmReturnToForm(formId);
        form_=0;
        id_=frmInvalidObjectId;
    }
    
    void Form::setBounds(const Rectangle& bounds)
    {
        assert(form_!=0);
        RectangleType native=toNative(bounds);
        WinSetBounds(windowHandle(), &native);
    }        


    void Form::setTitle(const String& title)
    {
        assert(form_!=0);
        FrmSetTitle(form_, "");
        title_=title;
        //! @bug On Sony Clie (OS 4.1) I experience bug described in Reference as corrected in post-OS 3.0... (Previous title is not erased)
        if (visible())
        {
            Graphics graph(windowHandle());
            Rectangle rect(bounds());
            rect.height()=18;
            graph.erase(rect);
        }        

        FrmSetTitle(form_, const_cast<char*>(title_.c_str()));            
    }
    
    const String& Form::title() const
    {
        assert(form_!=0);
        if (title_.empty())
            title_.assign(FrmGetTitle(form_));
        return title_;
    }
    
    bool Form::handleWindowExit(const struct _WinExitEventType& data)
    {
        const FormType* form=*this;
        if (data.exitWindow==static_cast<const void*>(form))
        {
        
            if (!application().runningOnTreo600())
            {
                UInt16 index = FrmGetFocus(form);
                if (noFocus != index)
                    entryFocusControlId_ = FrmGetObjectId(form, index);
                releaseFocus();
            }
            else 
                entryFocusControlId_ = frmInvalidObjectId;
                
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
        
}

