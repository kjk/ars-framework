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
                    if (0==form)
                        form=app.getOpenForm(FrmGetActiveFormID());
                    break;
                    
                case winEnterEvent:
                    form=app.getOpenForm(event->data.winEnter.enterWindow);
                    if (0!=form)
                        break;
                    
                default:
                    form=app.getOpenForm(FrmGetActiveFormID());
            }
            
        }
        assert(form!=0);
        return form->handleEvent(*event);
    }
    
    Form::Form(Application& app, UInt16 id):
        application_(app),
        id_(id),
        form_(0), 
        deleteOnClose(true)
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
        if (deleteOnClose) 
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
        deleteOnClose=false;
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
        deleteOnClose=false;
        handleClose();        
        FrmReturnToForm(formId);
    }
    
    void Form::setBounds(const Rectangle& bounds)
    {
        assert(form_!=0);
        RectangleType native=toNative(bounds);
        WinSetBounds(windowHandle(), &native);
    }        

}

