#include <Form.hpp>
#include <Application.hpp>
#include <Graphics.hpp>
#include <FormGadget.hpp>
#include <SysUtils.hpp>

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
        focusControlId_(frmInvalidObjectId)
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
    
    bool Form::handleEvent(EventType& event)
    {
        bool handled=false;
        switch (event.eType)
        {
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
            if (frmInvalidObjectId != focusControlId_)
            {
                FormObject object(*this, focusControlId_);
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
            UInt16 index = FrmGetFocus(form);
            if (noFocus != index)
                focusControlId_ = FrmGetObjectId(form, index);
            releaseFocus();
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
    
}

