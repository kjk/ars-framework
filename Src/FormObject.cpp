#include <FormObject.hpp>
#include <Graphics.hpp>
#include <DeviceInfo.hpp>
#include <FormGadget.hpp>
#include <68k/Hs.h>
#include <Application.hpp>

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
        assert(frmInvalidObjectId != id);
        index_ = FrmGetObjectIndex(*form_, id_ = id);
        assert(frmInvalidObjectId != index_);
        object_ = FrmGetObjectPtr(*form_, index_);
        assert(NULL != object_);
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
        assert(frmInvalidObjectId != index);
        id_ = FrmGetObjectId(*form_, index_ = index);
        object_ = FrmGetObjectPtr(*form_, index_);
        assert(0 != object_);
    }
    
    void FormObject::anchor(const Rectangle& boundingBox, FormObjectAnchorStyle horizAnchor, Coord rightMargin, FormObjectAnchorStyle vertAnchor, Coord bottomMargin)
    {
        Rectangle rect;
        bounds(rect);
        
        if (anchorLeftEdge == horizAnchor) 
            rect.x() = boundingBox.width() - rightMargin;
        else if (anchorRightEdge == horizAnchor)
            rect.width() = boundingBox.width() - rightMargin;
        
        if (anchorTopEdge == vertAnchor)
            rect.y() = boundingBox.height() - bottomMargin;
        else if (anchorBottomEdge == vertAnchor)
            rect.height() = boundingBox.height() - bottomMargin;
            
        setBounds(rect);
    }
    
    void FormObject::hide()
    {
        FrmHideObject(*form_, index_);
        if (frmGadgetObj == type())
        {
            FormGadget* gadget = static_cast<FormGadget*>(FrmGetGadgetData(*form_, index_));
            assert(NULL != gadget);
            gadget->usable_ = false;
            gadget->visible_ = false;
            gadget->notifyHide();      
        }
    }

    void FormObject::show()
    {
        if (frmGadgetObj == type())
        {
            FormGadget* gadget = static_cast<FormGadget*>(FrmGetGadgetData(*form_, index_));
            assert(NULL != gadget);
            gadget->usable_ = true;
            gadget->visible_ = true;            
            gadget->notifyShow();
        }
        FrmShowObject(*form_, index_);
    }
    
    bool FormObject::hasFocus() const
    {
        return index_ == form()->focusedControlIndex_;
    }
        
    void FormObject::focus()
    {
        assert(valid());
        if (form_->focusedControlIndex_ != index_)
            form_->releaseFocus();          // To make sure event focus lost is sent before event focus taken on Treo 600
        FormObjectKind kind = type();
        bool isTreo = form_->application().runningOnTreo600();
        bool isPastCobalt61 = (form_->application().romVersionMajor() > 6 || (form_->application().romVersionMajor() == 6 && form_->application().romVersionMinor() >= 1));
        if (isTreo && frmInvalidObjectId != id_)
            HsNavObjectTakeFocus(*form_, id_);
        if (!isTreo && (frmFieldObj == kind || frmTableObj == kind))
            FrmSetFocus(*form_, index_);
        else if (isPastCobalt61 && frmInvalidObjectId != id_)
            FrmSetFocus(*form_, index_);            
        form_->focusedControlIndex_ = index_;
    }
    
    void FormObject::draw()
    {
        assert(valid());
        FormObjectKind kind = type();
        switch (kind)
        {
            case frmFieldObj:
                FldDrawField(static_cast<FieldType*>(object_));
                break;
                
            case frmControlObj:
                CtlDrawControl(static_cast<ControlType*>(object_));
                break;
                
            case frmListObj:
                LstDrawList(static_cast<ListType*>(object_));
                break;
                
            case frmTableObj:
                TblDrawTable(static_cast<TableType*>(object_));
                break;
                
            case frmGadgetObj: {
                FormGadget* gadget = static_cast<FormGadget*>(FrmGetGadgetData(*form_, index_));
                assert(NULL != gadget);
                gadget->drawProxy();
                break;
            }
            
            case frmScrollBarObj:
                SclDrawScrollBar(static_cast<ScrollBarType*>(object_));
                break;
    
            default: // There's no other way to redraw other components 
                form_->draw();
        }        
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
    
    void Field::setText(MemHandle handle)
    {
        MemHandle oldHandle = FldGetTextHandle(object());
        FldSetTextHandle(object(), handle);
        if (NULL != oldHandle)
            MemHandleFree(oldHandle);
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
