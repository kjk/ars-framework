#ifndef __ARSLEXIS_FORM_HPP__
#define __ARSLEXIS_FORM_HPP__

#include "Debug.hpp"

namespace ArsLexis 
{

    class Application;
        
    class Form
    {
        Application& application_;
        UInt16 id_;
        FormType* form_;
        Boolean deleteOnClose;
        
        static Boolean routeEventToForm(EventType* event);
        
        Form(const Form&);
        Form& operator=(const Form&);
        
    protected:
    
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
        virtual void draw()
        {
            assert(form_!=0);
            FrmDrawForm(form_);
        }
        
        virtual Boolean handleEvent(EventType& event);
            
    public:
        
        Form(Application& app, UInt16 id);
        
        virtual Err initialize();
        
        virtual ~Form();
        
        Application& application()
        {return application_;}
        
        const Application& application() const
        {return application_;}
        
        void activate();
        
        UInt16 id() const 
        {return id_;}
        
        UInt16 showModal();
        
        /**
         * Queues @c frmUpdateEvent for this form.
         */
        void update(UInt16 updateCode=frmRedrawUpdateCode)
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
        
        operator const FormType* () const 
        {return form_;}
        
        operator FormType* ()
        {return form_;}
        
        Boolean visible() const
        {
            assert(form_);
            return FrmVisible(form_);
        }
        
        UInt16 getObjectIndex(UInt16 objectId) const
        {   
            assert(form_!=0);
            return FrmGetObjectIndex(form_, objectId);
        }
        
        void getObjectBounds(UInt16 index, RectangleType& bounds) const
        {
            assert(form_!=0);
            assert(frmInvalidObjectId!=index);
            FrmGetObjectBounds(form_, index, &bounds);
        }
        
        void setObjectBounds(UInt16 index, const RectangleType& bounds) const
        {
            assert(form_!=0);
            assert(frmInvalidObjectId!=index);
            FrmSetObjectBounds(form_, index, &bounds);
        }
        
        void getBounds(RectangleType& bounds) const
        {
            assert(form_!=0);
            FrmGetFormBounds(form_, &bounds);
        }
        
        void setBounds(const RectangleType& bounds);

        friend class Application;
    };
    
}

#endif