#ifndef __ARSLEXIS_FORM_HPP__
#define __ARSLEXIS_FORM_HPP__

#include "Debug.hpp"

#include <PalmOS.h>
#include <CWCallbackThunks.h>

namespace ArsLexis 
{

    class Application;
        
    class Form
    {
        Application& application_;
        UInt16 id_;
        FormType* form_;
        Boolean deleteOnClose;
        
        static Boolean routeEventToForm(EventType* event) throw();
        
        Form(const Form&);
        Form& operator=(const Form&);
        
    protected:
    
        FormType* form() throw()
        {return form_;}
        
        const FormType* form() const throw()
        {return form_;}

        /**
         * Handles @c frmCloseEvent.
         * This function calls <code>delete this</code>, so when overriding it should be called last, 
         * unless you know what you're doing.
         */
        virtual Boolean handleClose();
        
        /**
         * Handles @c frmOpenEvent.
         * Updates form and prevents calling FrmDeleteForm() in destructor.
         */
        virtual Boolean handleOpen();
        
        /**
         * Handles @c frmUpdateEvent.
         * Calls redraw().
         */
        virtual Boolean handleUpdate(UInt16 updateCode);
        
        /**
         * Redraws form.
         * Calls FrmDrawForm().
         */
        virtual void redraw();
            
    public:
        
        Form(Application& app, UInt16 id);
        
        virtual ~Form() throw();
        
        virtual Boolean handleEvent(EventType& event) throw();

        Application& application() throw()
        {return application_;}
        
        const Application& application() const throw()
        {return application_;}
        
        void activate() throw();
        
        UInt16 id() const throw() 
        {return id_;}
        
        Boolean isVisible() const throw() 
        {return FrmVisible(form());}
        
        UInt16 showModal() throw();
        
        /**
         * Queues @c frmUpdateEvent for this form.
         */
        void update(UInt16 updateCode=frmRedrawUpdateCode) throw()
        {FrmUpdateForm(id(), updateCode);}
        
        /**
         * Calls @c handleClose(), preventing a call to @c FrmDeleteForm() in destructor. 
         * Next calls @c FrmReturnToForm().
         * @note Keep in mind that after call to this function form is deleted. 
         */
        void returnToForm(UInt16 formId);

        /**
         * Calls @c returnToForm(0).
         */
        void closePopup()
        {returnToForm(0);}
        
        friend class Application;
    };

}

#endif