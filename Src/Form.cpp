#include "Form.hpp"
#include "Application.hpp"

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
        if (form)
            return form->handleEvent(*event);
        else
            return false;
    }
    
    Form::Form(Application& app, UInt16 id):
        application_(app),
        id_(id),
        form_(0), 
        deleteOnClose_(true)
    {
    }
    
    Err Form::initialize()
    {
        Err error=errNone;
        form_=FrmInitForm(id());
        if (form_)
        {
            FrmSetEventHandler(form_, application_.formEventHandlerThunk_);
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
        delete this;        
        return false;
    }
    
    bool Form::handleOpen()
    {
        deleteOnClose_=false;
        update();
        return true;
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
        title_=title;
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
            releaseFocus();
        return false;
    }
    
    void Form::update(UInt16 updateCode)
    {
        EventType event;
        MemSet(&event, sizeof(event), 0);        event.eType=frmUpdateEvent;
        event.data.frmUpdate.formID=id();
        event.data.frmUpdate.updateCode=updateCode;
        EvtAddUniqueEventToQueue(&event, 0, false);
    }

}

