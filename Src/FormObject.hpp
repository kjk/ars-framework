#ifndef __ARSLEXIS_FORM_OBJECT_HPP__
#define __ARSLEXIS_FORM_OBJECT_HPP__

#include <Form.hpp>

#ifdef __MWERKS__
#include <CWCallbackThunks.h>
#endif // __MWERKS__

namespace ArsLexis
{

    class Graphics;

#pragma mark FormObject

    enum FormObjectAnchorStyle {
        anchorNot,
        anchorLeftEdge,
        anchorTopEdge = anchorLeftEdge,
        anchorRightEdge,
        anchorBottomEdge = anchorRightEdge
    };
    
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
        
        Form* form()
        {return form_;}
        
        const Form* form() const
        {return form_;}

    public:

        void attach(UInt16 id);

        void attachByIndex(UInt16 index);

        explicit FormObject(Form& form, UInt16 id=frmInvalidObjectId);

        virtual ~FormObject();

        bool valid() const
        {return form_!=0 && (id_!=frmInvalidObjectId || frmGraffitiStateObj == type()) && index_!=frmInvalidObjectId && object_!=0;}

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
        
        void focus();
        
        bool focusable() const;

        void hide();

        void show();
        
        void draw();

        void anchor(const Rectangle& boundingBox, FormObjectAnchorStyle horizAnchor=anchorNot, Coord rightMargin=0, FormObjectAnchorStyle vertAnchor=anchorNot, Coord bottomMargin=0);
        
        void enableNavigation();
        
        friend class Form;
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
        
        ~ScrollBar();

    };

#pragma mark -
#pragma mark Field

    class Field: public FormObjectWrapper<FieldType>
    {
    
        void getAttribs(FieldAttrType& attr) const;
        
    public:

        explicit Field(Form& form, UInt16 id=frmInvalidObjectId):
            FormObjectWrapper(form, id)
        {}

        const char* text() const
        {return FldGetTextPtr(object());}

        void setText(const char* text)
        {return FldSetTextPtr(object(), const_cast<char*>(text));}
        
        void setText(const String& text)
        {setText(text.c_str());}

        void setText(MemHandle handle);
        
        status_t setEditableText(const char* text, uint_t length);
        
        status_t setEditableText(const String& text)
        {return setEditableText(text.data(), text.length());}

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
        
        bool usable() const;
        
        bool readOnly() const;
        
        bool numeric() const;
        
        
        
        ~Field();
        
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
        
        void setValue(int val)
        {CtlSetValue(object(), val);}
        
        int value() const
        {return CtlGetValue(object());}
        
        void setSliderValues(const uint_t* minVal, const uint_t* maxVal, const uint_t* pageSize, const uint_t* value)
        {
            CtlSetSliderValues(object(), 
                reinterpret_cast<const unsigned short*>(minVal), 
                reinterpret_cast<const unsigned short*>(maxVal),
                reinterpret_cast<const unsigned short*>(pageSize),
                reinterpret_cast<const unsigned short*>(value)
            );
        }
        
        void getSliderValues(uint_t& minVal, uint_t& maxVal, uint_t& pageSize, uint_t& value) const
        {
            CtlGetSliderValues(object(), 
                reinterpret_cast<unsigned short*>(&minVal), 
                reinterpret_cast<unsigned short*>(&maxVal),
                reinterpret_cast<unsigned short*>(&pageSize),
                reinterpret_cast<unsigned short*>(&value)
            );
        }
           
        void setLabel(const char* label)
        {CtlSetLabel(object(), label);}
        
        const char* label() const
        {return CtlGetLabel(object());}
        
        
        ~Control();
                
    };
    
#pragma mark -
#pragma mark List

    class List: public FormObjectWrapper<ListType>
    {
    public:

        explicit List(Form& form, UInt16 id = frmInvalidObjectId):
            FormObjectWrapper(form, id)
        {}
        
        void setChoices(const char* choices[], uint_t choicesCount)
        {LstSetListChoices(object(), const_cast<char**>(choices), choicesCount);}
        

        void setChoices(char* choices[], uint_t choicesCount)
        {
            LstSetListChoices(object(), const_cast<char**>(choices), choicesCount);
        }

        uint_t visibleItemsCount() const
        {return LstGetVisibleItems(object());}
        
        void draw()
        {LstDrawList(object());}
        
        void setSelection(int item)
        {LstSetSelection(object(), item);}

        int selection() const
        {return LstGetSelection(object());}

        void makeItemVisible(uint_t item)
        {LstMakeItemVisible(object(), item);}

        bool scroll(WinDirectionType direction, uint_t items)
        {return LstScrollList(object(), direction, items);}

        uint_t itemsCount() const
        {return LstGetNumberOfItems(object()); }
        
        int topItem() const
        {return LstGetTopItem(object());}

        void setSelectionDelta(int delta);
        
        class CustomDrawHandler {

            Form* form_;
            UInt16 listId_;

#ifdef __MWERKS__        
            _CW_CallbackThunk drawCallbackThunk_;
#endif
        
            static void drawCallback(Int16 itemNum, RectangleType* rect, Char** itemsText);
        
        public:
        
            CustomDrawHandler();
        
            virtual void drawItem(Graphics& graphics, List& list, uint_t item, const Rectangle& itemBounds)=0;
            
            virtual uint_t itemsCount() const=0;
            
            virtual ~CustomDrawHandler();

            friend class List;               
        };
        
        void setCustomDrawHandler(CustomDrawHandler* handler);
        
        void adjustVisibleItems();
        
        void setTopItem(uint_t item)
        {LstSetTopItem(object(), item);}
        
        void updateItemsCount(const CustomDrawHandler& handler);
        
        enum KeyHandlerOptions {
            optionScrollPagesWithLeftRight=1,
            optionFireListSelectOnCenter=2
        };
        /**
         * Performs selection change in case of up/down key or 5-way up/down or fires @c lstSelectEvent if 5-way center is pressed.
         * @param form @c Form object that gives acces to its 5-way test functions.
         * @param event @c keyDownEvent to handle.
         * @param scrollPagesWithLeftRight if set to @c true, will use left/right buttons to scroll page up/down.
         * @return @c true if list recognized event and handled it, @c false otherwise.
         */
        bool handleKeyDownEvent(const EventType& event, uint_t options=0);
        
        ~List();

    };
    
}

#endif
