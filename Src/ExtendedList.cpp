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
        assert(0==itemsCount());        return 0;
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



