#ifndef ARSLEXIS_FORM_GADGET_HPP__
#define ARSLEXIS_FORM_GADGET_HPP__

#include <FormObject.hpp>

namespace ArsLexis {

    class Graphics;
    
    class FormGadget: public FormObjectWrapper<FormGadgetType> {
        
        static Boolean gadgetHandler(FormGadgetTypeInCallback* gadget, UInt16 cmd, void* param);
        
        void setupGadget(FormType* form, UInt16 index);
        
        bool visible_;
        bool usable_;
        bool doubleBuffer_;

    protected:        

        void drawProxy();
        
        virtual void drawFocusRing() {}
        
        virtual void handleDraw(Graphics& graphics);
        
        virtual bool handleEvent(EventType& event);
        
        virtual bool handleGadgetCommand(UInt16 command, void* param);
        
        virtual bool handleEnter(const EventType& event); 
        
        virtual bool handleMiscEvent(const EventType& event);

        void setDoubleBuffer(bool val)
        {doubleBuffer_=val;}
        
        enum FocusChange {
            focusTaking,
            focusLosing
        };
        
        virtual void handleFocusChange(FocusChange change);
        
    public:
    
        bool visible() const
        {return visible_;}
        
        bool usable() const
        {return usable_;}
        
        explicit FormGadget(Form& form, UInt16 id=frmInvalidObjectId);
        
        ~FormGadget();

        //! Warning! this function overwrites non-virtual FormObject::attach(). 
        //! Take care not to call it through FormObject pointer or reference.
        void attach(UInt16 id);

        //! Warning! this function overwrites non-virtual FormObject::attachByIndex(). 
        //! Take care not to call it through FormObject pointer or reference.
        void attachByIndex(UInt16 index);
          
        friend class Form;
        friend class Application;
        friend class FormObject;
    };   

}

#endif