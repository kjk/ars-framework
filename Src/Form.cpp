#include "Form.hpp"
#include "Application.hpp"

namespace ArsLexis 
{

    Boolean Form::routeEventToForm(EventType* event) throw()
    {
        UInt16 formId=FrmGetActiveFormID();
        Application& app=Application::instance();
        Form* form=app.getOpenForm(formId);
        assert(form!=0);
        return form->handleEvent(*event);
    }
    
    void Form::activate() throw() 
    {
        assert(form_!=0);
        FrmSetActiveForm(form_);
    }
    
    Form::Form(Application& app, UInt16 id):
        application_(app),
        id_(id),
        form_(FrmInitForm(id)),
        deleteOnClose(true)
    {
        if (!form_) // This probably means we're out of memory or id is invalid. 
            throw std::bad_alloc(); // We can't easily detect which one is true, so assume out of memory.
        FrmSetEventHandler(form_, application_.formEventHandlerThunk_);
        try {
            application_.registerForm(id_, this);
        }
        catch (...)
        {
            FrmDeleteForm(form_);
            throw;
        }
    }
    
    Form::~Form() throw() 
    {
        if (deleteOnClose) 
            FrmDeleteForm(form_);
        application_.unregisterForm(id_);
    }
    
    Boolean Form::handleEvent(EventType& event) throw()
    {
        Boolean handled=false;
        switch (event.eType)
        {
            case frmOpenEvent:
                handled=handleOpen();
                break;
                
            case frmCloseEvent:
                handled=handleClose();
                break;
            
            case frmUpdateEvent:
                handled=handleUpdate(event.data.frmUpdate.updateCode);
                break;

        }
        return handled;
    }
    
    UInt16 Form::showModal() throw()
    {
        assert(form_!=0);
        UInt16 result=FrmDoDialog(form_);
        return result;
    }

    Boolean Form::handleClose() 
    {
        delete this;        
        return false;
    }
    
    Boolean Form::handleOpen()
    {
        deleteOnClose=false;
        update();
        return true;
    }
    
    Boolean Form::handleUpdate(UInt16 updateCode)
    {
        redraw();
        return true;
    }

    void Form::redraw()
    {
        FrmDrawForm(form_);
    }
    
    void Form::returnToForm(UInt16 formId)
    {
        deleteOnClose=false;
        handleClose();
        FrmReturnToForm(formId);
    }

}

