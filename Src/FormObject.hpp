#ifndef __ARSLEXIS_FORM_OBJECT_HPP__
#define __ARSLEXIS_FORM_OBJECT_HPP__

#include <Form.hpp>

namespace ArsLexis
{

#pragma mark FormObject

    class FormObject: private NonCopyable
    {
        Form* form_;
        UInt16 id_;
        UInt16 index_;
        void* object_;

    protected:

        void* wrappedObject()
        {return object_;}

        const void* wrappedObject() const
        {return object_;}

    public:

        void attach(UInt16 id);

        void attachByIndex(UInt16 index);

        explicit FormObject(Form& form, UInt16 id=frmInvalidObjectId);

        virtual ~FormObject();

        bool valid() const
        {return form_!=0 && id_!=frmInvalidObjectId && index_!=frmInvalidObjectId && object_!=0;}

        UInt16 index() const
        {return index_;}

        UInt16 id() const
        {return id_;}

        FormObjectKind type() const
        {return FrmGetObjectType(*form_, index_);}

        void bounds(Rectangle& rect) const;

        Rectangle bounds() const
        {
            Rectangle rect;
            bounds(rect);
            return rect;
        }

        void setBounds(const Rectangle& rect);

        void focus()
        {FrmSetFocus(*form_, index_);}

        void hide()
        {FrmHideObject(*form_, index_);}

        void show()
        {FrmShowObject(*form_, index_);}

    };
    
#pragma mark -
#pragma mark FormObjectWrapper<WrappedType>

    template<class WrappedType> 
    class FormObjectWrapper: public FormObject
    {
    protected:

        WrappedType* object()
        {return static_cast<WrappedType*>(wrappedObject());}

        const WrappedType* object() const
        {return static_cast<const WrappedType*>(wrappedObject());}

    public:

        FormObjectWrapper(Form& form, UInt16 id=frmInvalidObjectId):
            FormObject(form, id)
        {}

        operator WrappedType* ()
        {return object();}

        operator const WrappedType* () const
        {return object;}

    };

#pragma mark -
#pragma mark ScrollBar

    class ScrollBar: public FormObjectWrapper<ScrollBarType>
    {

    public:

        explicit ScrollBar(Form& form, UInt16 id=frmInvalidObjectId):
            FormObjectWrapper(form, id)
        {}

        void setPosition(int value, int min, int max, int pageSize)
        {SclSetScrollBar(object(), value, min, max, pageSize);}

    };

#pragma mark -
#pragma mark Field

    class Field: public FormObjectWrapper<FieldType>
    {
    public:

        explicit Field(Form& form, UInt16 id=frmInvalidObjectId):
            FormObjectWrapper(form, id)
        {}

        const char* text() const
        {return FldGetTextPtr(object());}

        void setText(const char* text)
        {return FldSetTextPtr(object(), const_cast<char*>(text));}

        void draw()
        {FldDrawField(object());}

        void setText(MemHandle handle)
        {FldSetTextHandle(object(), handle);}

        enum {npos=(uint_t)-1};

        void select(uint_t start=0, uint_t end=npos)
        {FldSetSelection(object(), start, npos==end?textLength():end);}

        uint_t textLength() const
        {return FldGetTextLength(object());}
        
        void erase(uint_t start=0, uint_t end=npos)
        {FldDelete(object(), start, npos==end?textLength():end);}
        
        bool insert(const char* text, uint_t length)
        {return FldInsert(object(), text, length);}
        
        uint_t maxLength() const
        {return FldGetMaxChars(object());}

        void replace(const char* text, uint_t length);
        
        void replace(const char* text)
        {replace(text, StrLen(text));}
        
        void replace(const String& text)
        {replace(text.data(), text.length());}
        
    };

#pragma mark -
#pragma mark Control

    class Control: public FormObjectWrapper<ControlType>
    {
    public:
    
        explicit Control(Form& form, UInt16 id=frmInvalidObjectId):
            FormObjectWrapper(form, id)
        {}
        
        void hit()
        {CtlHitControl(object());}

        void setEnabled(bool enabled)
        {CtlSetEnabled(object(), enabled);}
        
        bool enabled() const
        {return CtlEnabled(object());}
        
        void setGraphics(DmResID bitmapId, DmResID selBitmapId=NULL)
        {CtlSetGraphics(object(), bitmapId, selBitmapId);}
                
    };
    
#pragma mark -
#pragma mark List

    class List: public FormObjectWrapper<ListType>
    {
    public:

        explicit List(Form& form, UInt16 id=frmInvalidObjectId):
            FormObjectWrapper(form, id)
        {}
        
        void setChoices(const char** choices, uint_t choicesCount)
        {LstSetListChoices(object(), const_cast<char**>(choices), choicesCount);}
        
        uint_t visibleItems() const
        {return LstGetVisibleItems(object());}
        
        void draw()
        {LstDrawList(object());}
        
        void setSelection(uint_t item)
        {LstSetSelection(object(), item);}
        
        bool scroll(WinDirectionType direction, uint_t items)
        {return LstScrollList(object(), direction, items);}
    };
    
}

#endif