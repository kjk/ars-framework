#include <FormObject.hpp>
#include <Graphics.hpp>

namespace ArsLexis
{

    FormObject::FormObject(Form& form, UInt16 id):
        form_(&form),
        id_(frmInvalidObjectId),
        index_(frmInvalidObjectId),
        object_(0)
    {
        if (id!=frmInvalidObjectId)
            attach(id);
    }
    
    FormObject::~FormObject()
    {}
    
    void FormObject::attach(UInt16 id)
    {
        assert(frmInvalidObjectId!=id);
        index_=FrmGetObjectIndex(*form_, id_=id);
        assert(frmInvalidObjectId!=index_);
        object_=FrmGetObjectPtr(*form_, index_);
        assert(0!=object_);
    }
    
    void FormObject::bounds(Rectangle& out) const
    {
        RectangleType rect;
        FrmGetObjectBounds(*form_, index(), &rect);
        out=rect;
    }
    
    void FormObject::setBounds(const Rectangle& bounds)
    {
        assert(valid());
        RectangleType rect=toNative(bounds);
        FrmSetObjectBounds(*form_, index(), &rect);
    }

    void FormObject::attachByIndex(UInt16 index)
    {
        assert(frmInvalidObjectId!=index);
        id_=FrmGetObjectId(*form_, index_=index);
        if (frmGraffitiStateObj != type())
            assert(frmInvalidObjectId!=id_);
        object_=FrmGetObjectPtr(*form_, index_);
        assert(0!=object_);
    }
    
#pragma mark -

    ScrollBar::~ScrollBar()
    {}
    
    Field::~Field()
    {}
    
    Control::~Control()
    {}

    List::~List()
    {}
    
    void Field::replace(const char* text, uint_t length)
    {
        erase();
        insert(text, std::min(maxLength(), length));
    }
    
    status_t Field::setEditableText(const char* data, uint_t length)
    {
        MemHandle handle=MemHandleNew(length+1);
        if (NULL==handle)
            return memErrNotEnoughSpace;
        char* text=static_cast<char*>(MemHandleLock(handle));
        if (NULL==text)
            return memErrChunkNotLocked;
        StrNCopy(text, data, length);
        text[length]=chrNull;
        MemHandleUnlock(handle);
        setText(handle);
        return errNone;
    }

#pragma mark -
#pragma mark List


    // At any given time an element of the list is selected. This function
    // will set a new item selected of the pos curSelected + delta. delta
    // can be negative. If asked to set selection to a inexistent item (<0 or
    // >number of items) sets selection to the first/last item (this is so that
    // the caller doesn't have to worry about this stuff)
    void List::setSelectionDelta(int delta)
    {
        if (0==delta || 0==itemsCount())
            return;
        // Why? If some calculations lead to delta==0, let it be.
        // assert(0!=delta);
        int sel=selection();
        if (noListSelection==sel)
            sel = 0;
        sel+=delta;
        sel=std::max(0, std::min<int>(sel, itemsCount()-1));
        setSelection(sel);
        makeItemVisible(sel);
    }

    List::CustomDrawHandler::CustomDrawHandler():
        form_(0),
        listId_(frmInvalidObjectId)
#ifdef __MWERKS__        
        , drawCallbackThunk_(drawCallback)
#endif
    {}

    void List::updateItemsCount(const CustomDrawHandler& handler)
    {
        LstSetListChoices(object(), reinterpret_cast<Char**>(const_cast<CustomDrawHandler*>(&handler)), handler.itemsCount());
    }
   
    void List::setCustomDrawHandler(List::CustomDrawHandler* handler)
    {
        if (handler)
        {
            handler->form_=form();
            handler->listId_=id();
#ifdef __MWERKS__        
            LstSetDrawFunction(object(), reinterpret_cast<ListDrawDataFuncPtr>(&handler->drawCallbackThunk_.thunkData));
#else
            LstSetDrawFunction(object(), CustomDrawHandler::drawCallback);
#endif
            updateItemsCount(*handler);
        }
        else
        {
            LstSetDrawFunction(object(), 0);
            LstSetListChoices(object(), 0, 0);
        }
    }
    
    void List::CustomDrawHandler::drawCallback(Int16 itemNum, RectangleType* rect, Char** itemsText)
    {
        CustomDrawHandler* handler=reinterpret_cast<CustomDrawHandler*>(itemsText);
        assert(0!=handler);
        Rectangle bounds(*rect);
        List list(*handler->form_, handler->listId_);
        Graphics graphics(handler->form_->windowHandle());
        handler->drawItem(graphics, list, itemNum, bounds);
    }

    void List::adjustVisibleItems() 
    {
        int total=itemsCount();
        if (0==total) 
            return;
        int top=topItem();
        int visible=visibleItemsCount();
        if (total-top<visible)
            top=std::max(0, total-visible);
        setTopItem(top);
    }
 
    List::CustomDrawHandler::~CustomDrawHandler()
    {}


    bool List::handleKeyDownEvent(const EventType& event, uint_t options)
    {
        assert(valid());
        Form& form=*this->form();
        assert(keyDownEvent==event.eType);
        bool handled=false;
        int delta=0;
        int page=visibleItemsCount();
        if (form.fiveWayUpPressed(&event))
            delta=-1;
        else if (form.fiveWayDownPressed(&event))
            delta=1;
        else if ((optionScrollPagesWithLeftRight & options) && form.fiveWayLeftPressed(&event))
            delta=-page;
        else if ((optionScrollPagesWithLeftRight & options) && form.fiveWayRightPressed(&event))
            delta=page;
        else if ((optionFireListSelectOnCenter & options) && form.fiveWayCenterPressed(&event))
        {
            EventType e;
            MemSet(&e, sizeof(e), 0);
            if (noListSelection!=(e.data.lstSelect.selection=selection()))
            {
                e.eType=lstSelectEvent;
                e.data.lstSelect.listID=id();
                e.data.lstSelect.pList=object();
                EvtAddEventToQueue(&e);
                handled = true;
            }
        }
        else {
            switch (event.data.keyDown.chr)
            {
                case chrPageDown:
                    delta=page;
                    break;
                    
                case chrPageUp:
                    delta=-page;
                    break;
                
                case chrDownArrow:
                    delta=1;
                    break;

                case chrUpArrow:
                    delta=-1;
                    break;
                    
                case chrLeftArrow:
                    if (optionScrollPagesWithLeftRight & options)
                        delta=-page;
                    break;
                    
                case chrRightArrow:
                    if (optionScrollPagesWithLeftRight & options)
                        delta=page;
                    break;
            }
        }
        if (0!=delta)
        {
            setSelectionDelta(delta);
            handled=true;
        }
        return handled;
    }

}
