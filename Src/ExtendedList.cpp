#include <ExtendedList.hpp>
#include <Graphics.hpp>

#ifdef __MWERKS__
#pragma pcrelconstdata on
#endif

using namespace ArsLexis;

ExtendedList::ExtendedList(Form& form, UInt16 id):
    FormGadget(form, id),
    itemRenderer_(0),
    selection_(noListSelection),
    topItem_(noListSelection),
    itemHeight_(18),
    scrollBarWidth_(7),
    scrollButtonHeight_(7)
{
    setRgbColor(itemBackground_, 76, 124, 189);
    setRgbColor(selectedItemBackground_, 87, 152, 211);
//    setRgbColor(listBackground_, 65, 97, 166);
    setRgbColor(listBackground_, 255, 255, 255);
//    WinSetBackColorRGB(0, &listBackground_);
    setRgbColor(foreground_, 255, 255, 255);

    
//    UInt32 screenDepths=1;
//    Boolean color=false;
//    Err error=WinScreenMode(winScreenModeGet, NULL, NULL, &screenDepths, &color);
}

ExtendedList::~ExtendedList()
{}
    
void ExtendedList::drawItem(Graphics& graphics, const Rectangle& bounds, uint_t item, bool selected)
{
    assert(item<itemsCount());
    assert(0!=itemRenderer_);
    RGBColorType oldColor;
    WinSetBackColorRGB(selected?&selectedItemBackground_:&itemBackground_, &oldColor);
    Rectangle rect=bounds;
    drawItemBackground(graphics, rect, item, selected);
    itemRenderer_->drawItem(graphics, *this, item, rect);
    WinSetBackColorRGB(&oldColor, 0);
}

void ExtendedList::drawItemProxy(Graphics& graphics, const Rectangle& listBounds, uint_t item)
{
    assert(noListSelection!=topItem_);
    assert(topItem_<=item);
    uint_t offset=item-topItem_;
    Rectangle itemBounds(listBounds.x(), listBounds.y()+offset*itemHeight_, listBounds.width()-visibleScrollBarWidth(), itemHeight_);
    Rectangle clipRectangle(itemBounds);
    if (clipRectangle.y()+clipRectangle.height()>listBounds.y()+listBounds.height())
    {
        clipRectangle.height()=listBounds.y()+listBounds.height()-clipRectangle.y();
        assert(clipRectangle.height()>0);
    }
    Graphics::ClipRectangleSetter setClip(graphics, clipRectangle);
    bool selected=(selection_==item);
    drawItem(graphics, itemBounds, item, selected);    
}

void ExtendedList::draw(Graphics& graphics)
{
    Rectangle listBounds;
    bounds(listBounds);
    drawBackground(graphics, listBounds);
    const uint_t itemsCount=this->itemsCount();
    if (0==itemsCount)
        return;
    assert(noListSelection!=topItem_);
    assert(itemsCount>topItem_);
    uint_t itemsToDisplay=listBounds.height()/itemHeight_;
    if (0 != listBounds.height()%itemHeight_)
        ++itemsToDisplay;
    uint_t itemsBelow=itemsCount-topItem_;
    itemsToDisplay=std::min(itemsToDisplay, itemsBelow);
    
    RGBColorType oldFore, oldText;
    WinSetForeColorRGB(&foreground_, &oldFore);
    WinSetTextColorRGB(&foreground_, &oldText);
    for (uint_t i=0; i<itemsToDisplay; ++i)
        drawItemProxy(graphics, listBounds, topItem_+i);
    WinSetTextColorRGB(&oldText, 0);
    WinSetForeColorRGB(&oldFore, 0);
    listBounds.x()=listBounds.x()+listBounds.width()-visibleScrollBarWidth();
    Graphics::ClipRectangleSetter setClip(graphics, listBounds);
    drawScrollBar(graphics, listBounds);
}

uint_t ExtendedList::height() const
{   
    RectangleType rect;
    FrmGetObjectBounds(*form(), index(), &rect);
    return rect.extent.y;
}

uint_t ExtendedList::visibleItemsCount() const
{
    if (noListSelection==topItem_) 
    {
        assert(0==itemsCount());
        return 0;
    }
    uint_t itemsCount=this->itemsCount();
    uint_t itemsBelow=itemsCount-topItem_;
    uint_t listHeight=height();
    uint_t visibleCount=listHeight/itemHeight_;
    if (0 !=listHeight%itemHeight_)
        ++visibleCount;
    visibleCount=std::min(visibleCount, itemsBelow);
    return visibleCount;
}

void ExtendedList::setTopItem(uint_t item, RedrawOption ro)
{
    assert(item<itemsCount());
    topItem_=item;
    if (redraw==ro)
        draw();        
}

void ExtendedList::setItemHeight(uint_t h, RedrawOption ro)
{
    assert(h>0);
    assert(h<height());
    itemHeight_=h;
    if (redraw==ro)
        draw();
}

void ExtendedList::setSelection(int item, RedrawOption ro)
{
    int oldSelection=selection_;
    bool scrolled=false;
    if (noListSelection!=(selection_=item)) 
    {
        uint_t itemsCount=this->itemsCount();
        assert(item<itemsCount);
        assert(noListSelection!=topItem_);
        if (item<topItem_)
        {
            topItem_=item;
            scrolled=true;
        }
        else 
        {
            uint_t listHeight=height();
            uint_t viewCapacity=listHeight/itemHeight_;
            if (item>=topItem_+viewCapacity)
            {
                topItem_=item-viewCapacity+1;
                scrolled=true;
            }
        }     
    }
    if (redraw==ro)
    {
        if (scrolled)
            draw();
        else 
        {
            Graphics graphics(form()->windowHandle());
            Rectangle listBounds;
            bounds(listBounds);

            RGBColorType oldFore, oldText;
            WinSetForeColorRGB(&foreground_, &oldFore);
            WinSetTextColorRGB(&foreground_, &oldText);
            if (noListSelection!=oldSelection)
                drawItemProxy(graphics, listBounds, oldSelection);
            if (noListSelection!=selection_)
                drawItemProxy(graphics, listBounds, selection_);
            WinSetTextColorRGB(&oldText, 0);
            WinSetForeColorRGB(&oldFore, 0);
        }
    }
}

void ExtendedList::setItemRenderer(ItemRenderer* itemRenderer, RedrawOption ro)
{
    if (0!=(itemRenderer_=itemRenderer) && 0!=itemRenderer->itemsCount())
        topItem_=0;
    else 
        topItem_=noListSelection;
    selection_=noListSelection;
    if (redraw==ro)
        draw();
}

void ExtendedList::adjustVisibleItems(RedrawOption ro) 
{
    int total=itemsCount();
    if (0==total) 
        return;
    int visible=visibleItemsCount();
    if (total-topItem_<visible)
    {
        topItem_=std::max(0, total-visible);
        if (redraw==ro)
            draw();
    }
}

namespace {

    static void saturate(UInt8& val, int level)
    {
        if (level>0)
        {
            if (255-level>val)
                val+=level;
            else
                val=255;
        }
        else
        {
            if (0-level<val)
                val+=level;
            else
                val=0;
        }
    }
    
    static void saturate(RGBColorType& rgb, int level)
    {
        saturate(rgb.r, level);
        saturate(rgb.g, level);
        saturate(rgb.b, level);
    }
    
}

void ExtendedList::drawItemBackground(Graphics& graphics, Rectangle& bounds, uint_t item, bool selected)
{
    RGBColorType oldColor;
    
    RGBColorType bgColor=(selected?selectedItemBackground_:itemBackground_);
    RGBColorType tmp=bgColor;
    saturate(tmp, 20);
    WinSetForeColorRGB(&tmp, &oldColor);
    const uint_t x0=bounds.x();
    const uint_t y0=bounds.y();
    const uint_t x1=x0+bounds.width()-1;
    const uint_t y1=y0+bounds.height()-1;
    graphics.drawLine(x0+1, y0, x1-1, y0);
    graphics.drawLine(x0, y0+1, x0, y1-2);
    graphics.drawLine(x0, y0+1, x0+1, y0+1);
    tmp=bgColor;
    saturate(tmp, 10);
    WinSetForeColorRGB(&tmp, 0);
    graphics.drawLine(x0+2, y0+1, x1-2, y0+1);
    graphics.drawLine(x0+1, y0+2, x0+1, y1-3);
    tmp=bgColor;
    saturate(tmp, -20);
    WinSetForeColorRGB(&tmp, 0);
    graphics.drawLine(x0+1, y1-1, x1-1, y1-1);
    graphics.drawLine(x1, y0+1, x1, y1-2);
    graphics.drawLine(x1-1, y1-2, x1, y1-2);
    tmp=bgColor;
    saturate(tmp, -10);
    WinSetForeColorRGB(&tmp, 0);
    graphics.drawLine(x0+1, y1-2, x1-2, y1-2);
    graphics.drawLine(x1-1, y0+1, x1-1, y1-3);
    WinSetForeColorRGB(&oldColor, 0);
    bounds.explode(2, 2, -4, -5);
    graphics.erase(bounds);
    
}

void ExtendedList::drawBackground(Graphics& graphics, const Rectangle& bounds)
{
    RGBColorType oldColor;
    WinSetBackColorRGB(&listBackground_, &oldColor);
    graphics.erase(bounds);
    WinSetBackColorRGB(&oldColor, 0);
}

void ExtendedList::drawScrollBar(Graphics& graphics, const Rectangle& bounds)
{
}

ExtendedList::ItemRenderer::ItemRenderer()
{}

ExtendedList::ItemRenderer::~ItemRenderer()
{}

bool ExtendedList::handleKeyDownEvent(const EventType& event, uint_t options)
{
    assert(valid());
    Form& form=*this->form();
    assert(keyDownEvent==event.eType);
    bool handled=false;
    int delta=0;
    int page=height()/itemHeight_;
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
        if (noListSelection!=selection_)
        {
            fireItemSelectEvent();
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

void ExtendedList::setSelectionDelta(int delta, RedrawOption ro)
{
    uint_t itemsCount=0;
    if (0==delta || 0==(itemsCount=this->itemsCount()))
        return;
    int sel=(noListSelection==selection_)?0:selection_;
    sel+=delta;
    sel=std::max(0, std::min<int>(sel, itemsCount-1));
    selection_=sel;
    if (redraw==ro)
        draw();
}

bool ExtendedList::handleEvent(EventType& event)
{
    bool handled=false;
    switch (event.eType) 
    {
        case penMoveEvent:
            handlePenMove(event);
            handled=true;
            break;
        
        case penUpEvent:
            handlePenUp(event);
            handled=true;
            break;
            
        default:
            handled=FormGadget::handleEvent(event);            
    }
    return handled;
}

bool ExtendedList::handleEnter(const EventType& event)
{
    Point penPos(event.screenX, event.screenY);
    WinDisplayToWindowPt(reinterpret_cast<Int16*>(&penPos.x), reinterpret_cast<Int16*>(&penPos.y));
    Rectangle bounds;
    this->bounds(bounds);
    assert(bounds && penPos);
    Rectangle itemsBounds=bounds;
    itemsBounds.width()-=visibleScrollBarWidth();
    if (itemsBounds && penPos)
        handlePenInItemsList(bounds, penPos, false);
    else 
        handlePenInScrollBar(bounds, penPos, false);
    return true;
}

void ExtendedList::handlePenInItemsList(const Rectangle& bounds, const Point& penPos, bool penUp)
{
    uint_t itemsCount=0;
    if (0==(itemsCount=this->itemsCount()))
        return;
    assert(noListSelection!=topItem_);
    assert(topItem_<itemsCount);
    uint_t item=topItem_+(penPos.y-bounds.y())/itemHeight_;
    if (selection_!=item && item<itemsCount)
        setSelection(item, redraw);
    if (penUp && item<itemsCount)
        fireItemSelectEvent();
}

void ExtendedList::handlePenInScrollBar(const Rectangle& bounds, const Point& penPos, bool penUp)
{
    //! @todo handle pen in scrollbar
}

void ExtendedList::handlePenUp(const EventType& event)
{
    Point penPos(event.screenX, event.screenY);
    WinDisplayToWindowPt(reinterpret_cast<Int16*>(&penPos.x), reinterpret_cast<Int16*>(&penPos.y));
    Rectangle bounds;
    this->bounds(bounds);
    if (!(bounds && penPos))
        return;
    Rectangle itemsBounds=bounds;
    itemsBounds.width()-=visibleScrollBarWidth();
    if (itemsBounds && penPos)
        handlePenInItemsList(bounds, penPos, true);
    else
        handlePenInScrollBar(bounds, penPos, true);
}

void ExtendedList::handlePenMove(const EventType& event)
{
    Point penPos(event.screenX, event.screenY);
    WinDisplayToWindowPt(reinterpret_cast<Int16*>(&penPos.x), reinterpret_cast<Int16*>(&penPos.y));
    Rectangle bounds;
    this->bounds(bounds);
    if (!(bounds && penPos))
    {
        //! @todo add scrolling when pen moves outside of list
    }
    else
    {
        Rectangle itemsBounds=bounds;
        itemsBounds.width()-=visibleScrollBarWidth();
        if (itemsBounds && penPos)
            handlePenInItemsList(bounds, penPos, false);
        else
            handlePenInScrollBar(bounds, penPos, false);
    }    
}

void ExtendedList::fireItemSelectEvent()
{
    EventType event;
    MemSet(&event, sizeof(event), 0);
    event.eType=lstSelectEvent;
    event.data.lstSelect.listID=id();
    event.data.lstSelect.pList=reinterpret_cast<ListType*>(object());
    event.data.lstSelect.selection=selection_;
    EvtAddEventToQueue(&event);
}
        
bool ExtendedList::scrollBarVisible() const
{
    uint_t itemsCount=0;
    if (0==(itemsCount=this->itemsCount()))
        return false;
    uint_t viewCapacity=height()/itemHeight_;
    return itemsCount>viewCapacity;
}





