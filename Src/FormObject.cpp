#include <FormObject.hpp>

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
        if (0==delta)
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
        handler->drawItem(list, itemNum, bounds);
    }

    void List::adjustVisibleItems() 
    {
        int top=topItem();
        int visible=visibleItemsCount();
        int total=itemsCount();
        if (total-top<visible)
            top=std::max(0, total-visible);
        setTopItem(top);
    }
 
    List::CustomDrawHandler::~CustomDrawHandler()
    {}

}
