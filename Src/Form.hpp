#ifndef __ARSLEXIS_FORM_HPP__
#define __ARSLEXIS_FORM_HPP__

#include "Debug.hpp"
#include "BaseTypes.hpp"
#include "Geometry.hpp"

namespace ArsLexis 
{

    class Application;
        
    class Form
    {
        Application& application_;
        UInt16 id_;
        FormType* form_;
        bool deleteOnClose_;
        mutable String title_;
        
        static Boolean routeEventToForm(EventType* event);
        
        Form(const Form&);
        Form& operator=(const Form&);
        
    protected:
    
        /**
         * Handles @c frmCloseEvent.
         * This function calls <code>delete this</code>, so when overriding it should be called last, 
         * unless you know what you're doing.
         */
        virtual bool handleClose();
        
        /**
         * Handles @c frmOpenEvent.
         * Updates form and prevents calling FrmDeleteForm() in destructor.
         */
        virtual bool handleOpen();
        
        /**
         * Handles @c frmUpdateEvent.
         * Calls redraw().
         */
        virtual bool handleUpdate(UInt16 updateCode);
        
        /**
         * Redraws form.
         * Calls FrmDrawForm().
         */
        virtual void draw(UInt16 updateCode=frmRedrawUpdateCode)
        {FrmDrawForm(form_);}
        
        virtual bool handleWindowEnter(const struct _WinEnterEventType&)
        {return false;}
        
        virtual bool handleWindowExit(const struct _WinExitEventType& data);
        
        virtual bool handleEvent(EventType& event);
        
        virtual bool handleMenuCommand(UInt16)
        {return false;}
            
    public:
        
        Form(Application& app, UInt16 id);
        
        virtual Err initialize();
        
        virtual ~Form();
        
        WinHandle windowHandle() const
        {return FrmGetWindowHandle(form_);}
        
        Application& application()
        {return application_;}
        
        const Application& application() const
        {return application_;}
        
        void activate()
        {FrmSetActiveForm(form_);}
        
        UInt16 id() const 
        {return id_;}
        
        UInt16 showModal();
        
        /**
         * Queues @c frmUpdateEvent for this form.
         */
        void update(UInt16 updateCode=frmRedrawUpdateCode);
        
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
        
        bool visible() const
        {return FrmVisible(form_);}
        
        void bounds(Rectangle& out) const
        {
            RectangleType rect;
            FrmGetFormBounds(form_, &rect);
            out=rect;
        }
        
        Rectangle bounds() const
        {
            Rectangle rect;
            bounds(rect);
            return rect;
        }
        
        void setBounds(const Rectangle& bounds);

        UInt16 focusedObject() const
        {return FrmGetFocus(form_);}
        
        void releaseFocus()
        {FrmSetFocus(form_, noFocus);}
        
        void setTitle(const String& title);
        const String& title() const;

        friend class Application;
    };
    
}

#endif