#include <ExtendedList.hpp>
#include <Graphics.hpp>

using namespace ArsLexis;

ExtendedList::ExtendedList(Form& form, UInt16 id):
    FormGadget(form, id),
    itemRenderer_(0),
    selection_(noListSelection),
    topItem_(noListSelection),
    itemHeight_(12),
    scrollBarWidth_(7)
{
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
    drawItemBackground(graphics, bounds, item, selected);
    itemRenderer_->drawItem(graphics, *this, item, bounds);
}

void ExtendedList::drawItemProxy(Graphics& graphics, const Rectangle& listBounds, uint_t item)
{
    assert(noListSelection!=topItem_);
    assert(topItem_<=item);
    uint_t offset=item-topItem_;
    Rectangle itemBounds(listBounds.x(), listBounds.y()+offset*itemHeight_, listBounds.width()-scrollBarWidth_, itemHeight_);
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
    for (uint_t i=0; i<itemsToDisplay; ++i)
        drawItemProxy(graphics, listBounds, topItem_+i);
    listBounds.x()=listBounds.x()+listBounds.width()-scrollBarWidth_;
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
            if (noListSelection!=oldSelection)
                drawItemProxy(graphics, listBounds, oldSelection);
            if (noListSelection!=selection_)
                drawItemProxy(graphics, listBounds, selection_);
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

void ExtendedList::drawItemBackground(Graphics& graphics, const Rectangle& bounds, uint_t item, bool selected)
{
}

void ExtendedList::drawBackground(Graphics& graphics, const Rectangle& bounds)
{
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
        EventType e;
        MemSet(&e, sizeof(e), 0);
/*        
        if (noListSelection!=(e.data.lstSelect.selection=selection()))
        {
            e.eType=lstSelectEvent;
            e.data.lstSelect.listID=id();
            e.data.lstSelect.pList=object();
            EvtAddEventToQueue(&e);
            handled = true;
        }
*/        
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




