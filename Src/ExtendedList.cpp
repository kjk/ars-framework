#include <ExtendedList.hpp>
#include <Graphics.hpp>
#include <DeviceInfo.hpp>
#include <68k/Hs.h>
#include <Application.hpp>
#include <SysUtils.hpp>

#ifdef __MWERKS__
#pragma pcrelconstdata on
#endif

using namespace ArsLexis;


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

    static void drawBevel(Graphics& graphics, Rectangle& bounds, const RGBColorType& color, bool doubleDensity)
    {
        RGBColorType oldColor;
        RGBColorType tmp=color;
        saturate(tmp, 80);
        WinSetForeColorRGB(&tmp, &oldColor);
        uint_t x0=bounds.x();
        uint_t y0=bounds.y();
        uint_t x1=x0+bounds.width()-1;
        uint_t y1=y0+bounds.height();
        UInt16 origCoordinateSystem;
        if (doubleDensity) 
        {
            x0 *= 2;
            y0 *= 2;
            x1 *= 2;
            x1 += 1;
            y1 *= 2;
            origCoordinateSystem = WinSetCoordinateSystem(kCoordinatesNative);
        }
        graphics.drawLine(x0+1, y0, x1-1, y0);
        graphics.drawLine(x0, y0+1, x0, y1-2);
        graphics.drawLine(x0, y0+1, x0+1, y0+1);
        tmp=color;
        saturate(tmp, 20);
        WinSetForeColorRGB(&tmp, 0);
        graphics.drawLine(x0+2, y0+1, x1-2, y0+1);
        graphics.drawLine(x0+1, y0+2, x0+1, y1-3);
        tmp=color;
        saturate(tmp, -80);
        WinSetForeColorRGB(&tmp, 0);
        graphics.drawLine(x0+1, y1-1, x1-1, y1-1);
        graphics.drawLine(x1, y0+1, x1, y1-2);
        graphics.drawLine(x1-1, y1-2, x1, y1-2);
        tmp=color;
        saturate(tmp, -20);
        WinSetForeColorRGB(&tmp, 0);
        graphics.drawLine(x0+1, y1-2, x1-2, y1-2);
        graphics.drawLine(x1-1, y0+1, x1-1, y1-3);
        WinSetForeColorRGB(&oldColor, 0);
        WinSetBackColorRGB(&color, &oldColor);
        if (doubleDensity)
        {
            Rectangle fore(x0+2, y0+2, x1-x0-3, y1-y0-4);
            graphics.erase(fore);
            WinSetCoordinateSystem(origCoordinateSystem);
            bounds.explode(1, 1, -2, -2);
        }
        else
        {
            bounds.explode(2, 2, -4, -4);
            graphics.erase(bounds);
        }
        WinSetBackColorRGB(&oldColor, 0);
    }
    
    inline static bool rgbEqual(const RGBColorType& c1, const RGBColorType& c2)
    {   
        return c1.r==c2.r && c1.g==c2.g && c1.b==c2.b;
    }
}

ExtendedList::ExtendedList(Form& form, UInt16 id):
    FormGadget(form, id),
    itemRenderer_(0),
    selection_(noListSelection),
    topItem_(noListSelection),
    itemHeight_(18),
    scrollBarWidth_(12),
    scrollButtonHeight_(12),
    hasHighDensityFeatures_(false),
    screenIsDoubleDensity_(false),
    windowSettingsChecked_(false),
    trackingScrollbar_(false),
    notifyChangeSelection_(false),
    topItemBeforeTracking_(noListSelection),
    upBitmapId_(frmInvalidObjectId),
    downBitmapId_(frmInvalidObjectId),
    scheduledScrollDirection_(scheduledScrollAbandoned),
    lastItemsCount_(0)
{
    UInt32 version;
    Err error=FtrGet(sysFtrCreator, sysFtrNumWinVersion, &version);
    if (errNone==error && 4<=version)
        hasHighDensityFeatures_=true;

    /*setRgbColor(listBackgroundColor, 255, 255, 255);
    setRgbColor(itemBackgroundColor, 70, 163, 255);
    setRgbColor(selectedItemBackgroundColor, 0, 107, 215);
    setRgbColor(foregroundColor, 255, 255, 255);*/

    setRgbColor(listBackgroundColor, 255, 255, 255);
    setRgbColor(itemBackgroundColor, 156, 207, 206);
    setRgbColor(selectedItemBackgroundColor, 99, 154, 206);
    setRgbColor(noFocusItemBackgroundColor, 99, 154, 206);
    setRgbColor(foregroundColor, 0, 0, 0);
    setRgbColor(selectedForegroundColor, 0, 0, 0);
    
}

ExtendedList::~ExtendedList()
{}
    
void ExtendedList::drawItem(Graphics& graphics, const Rectangle& bounds, uint_t item, bool selected)
{
    assert(item<itemsCount());
    assert(0!=itemRenderer_);
    RGBColorType oldColor;
    if (selected)
    {
        if (form()->application().runningOnTreo600() && !hasFocus())
            WinSetBackColorRGB(&noFocusItemBackgroundColor, &oldColor);
        else
            WinSetBackColorRGB(&selectedItemBackgroundColor, &oldColor);
    }
    else
        WinSetBackColorRGB(&itemBackgroundColor, &oldColor);
    Rectangle rect=bounds;
    drawItemBackground(graphics, rect, item, selected);
    itemRenderer_->drawItem(graphics, *this, item, rect);
    WinSetBackColorRGB(&oldColor, 0);
}

void ExtendedList::drawItemProxy(Graphics& graphics, const Rectangle& listBounds, uint_t item, bool showScrollbar)
{
    if (noListSelection==topItem_)
        topItem_=0;
    assert(topItem_<=item);
    uint_t offset=item-topItem_;
    Rectangle itemBounds(listBounds.x(), listBounds.y()+offset*itemHeight_, showScrollbar?listBounds.width()-scrollBarWidth_:listBounds.width(), itemHeight_);
    Rectangle clipRectangle(itemBounds);
    if (clipRectangle.y()+clipRectangle.height()>listBounds.y()+listBounds.height())
    {
        clipRectangle.height()=listBounds.y()+listBounds.height()-clipRectangle.y();
        assert(clipRectangle.height()>0);
    }
    Graphics::ClipRectangleSetter setClip(graphics, clipRectangle);
    bool selected = false;
    
    RGBColorType newFore;
    
    if (selection_==item)
    {
        newFore = selectedForegroundColor;
        selected = true;
    }
    else
        newFore = foregroundColor;
        
    RGBColorType oldFore, oldText;
    WinSetForeColorRGB(&newFore, &oldFore);
    WinSetTextColorRGB(&newFore, &oldText);

    drawItem(graphics, itemBounds, item, selected);    
    
    WinSetTextColorRGB(&oldText, 0);
    WinSetForeColorRGB(&oldFore, 0);
    
}

void ExtendedList::handleDraw(Graphics& graphics)
{
    uint_t itemsCount = this->itemsCount();
    if (lastItemsCount_ != itemsCount)
    {
        notifyItemsChanged();
        lastItemsCount_ = itemsCount;
    }
//    if (false)  // temporary, to disable over-riding colors set in the constructor
    if (!windowSettingsChecked_)
    {
        windowSettingsChecked_=true;
/*        
        WinIndexToRGB(UIColorGetTableEntryIndex(UIFormFill), &listBackgroundColor);
        WinIndexToRGB(UIColorGetTableEntryIndex(UIMenuSelectedForeground), &foregroundColor);
        WinIndexToRGB(UIColorGetTableEntryIndex(UIMenuSelectedFill), &itemBackgroundColor);
        WinIndexToRGB(UIColorGetTableEntryIndex(UIMenuForeground), &selectedItemBackgroundColor);
        if (rgbEqual(itemBackgroundColor, selectedItemBackgroundColor))
        {
            saturate(selectedItemBackgroundColor, 64);
            if (rgbEqual(itemBackgroundColor, selectedItemBackgroundColor))
                saturate(selectedItemBackgroundColor, -64);
        }
*/        
        if (hasHighDensityFeatures_)
        {
            UInt32 attr;
            Err error=WinScreenGetAttribute(winScreenDensity, &attr);
            if (errNone==error && kDensityDouble==attr)
                screenIsDoubleDensity_=true;

            setDoubleBuffer(true);

        }
    }
    Rectangle listBounds;
    bounds(listBounds);
    drawBackground(graphics, listBounds);
    if (0 == itemsCount)
        return;
    if (noListSelection == topItem_)
        topItem_=0;
    assert(itemsCount > topItem_);
    uint_t viewCapacity=listBounds.height()/itemHeight_;
    uint_t itemsToDisplay=viewCapacity;
//    if (0 != listBounds.height() % itemHeight_)
//        ++itemsToDisplay;
    uint_t itemsBelow=itemsCount-topItem_;
    bool showScrollbar=(itemsCount>viewCapacity);
    itemsToDisplay=std::min(itemsToDisplay, itemsBelow);
    for (uint_t i=0; i<itemsToDisplay; ++i)
        drawItemProxy(graphics, listBounds, topItem_+i, showScrollbar);
    if (showScrollbar)
    {
        listBounds.x()=listBounds.x()+listBounds.width()-visibleScrollBarWidth();
        Graphics::ClipRectangleSetter setClip(graphics, listBounds);
        drawScrollBar(graphics, listBounds);
    }
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
    uint_t itemsBelow=itemsCount - topItem_;
    uint_t listHeight=height();
    uint_t visibleCount=listHeight/itemHeight_;
//    if (0 != listHeight  %itemHeight_)
//        ++visibleCount;
    visibleCount=std::min(visibleCount, itemsBelow);
    return visibleCount;
}

void ExtendedList::setTopItem(uint_t item, RedrawOption ro)
{
    assert(item<itemsCount());
    topItem_=item;
    if (redraw==ro)
        drawProxy();        
}

void ExtendedList::setItemHeight(uint_t h, RedrawOption ro)
{
    assert(h>0);
    assert(h<height());
    itemHeight_=h;
    if (redraw==ro)
        drawProxy();
}

void ExtendedList::setChangeSelectionNotification(bool notify)
{
    notifyChangeSelection_ = notify;
}

void ExtendedList::setSelection(int item, RedrawOption ro)
{
    int oldSelection=selection_;
    if (notifyChangeSelection_)
    {
        if (oldSelection != item)
        {
            sendEvent(selChangedEvent);
        }    
    }    
    bool scrolled=false;
    bool showScrollbar=false;
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
            if (viewCapacity<itemsCount)
                showScrollbar=true;
        }     
    }
    if (redraw == ro)
        drawProxy();
}

void ExtendedList::drawFocusRing()
{
    assert(hasFocus());
    assert(form()->application().runningOnTreo600());
    RectangleType rect;
    FrmGetObjectBounds(*form(), index(), &rect);
    Err error = HsNavDrawFocusRing(*form(), id(), hsNavFocusRingNoExtraInfo, &rect, hsNavFocusRingStyleSquare, false);
    assert(errNone == error);
}

void ExtendedList::setItemRenderer(ItemRenderer* itemRenderer, RedrawOption ro)
{
    if (0!=(itemRenderer_=itemRenderer) && 0!=itemRenderer->itemsCount())
        topItem_=0;
    else 
        topItem_=noListSelection;
    selection_=noListSelection;
    if (redraw==ro)
        drawProxy();
}

void ExtendedList::adjustVisibleItems(RedrawOption ro) 
{
    int total=itemsCount();
    if (0==total) 
        return;
    int visible=height()/itemHeight_;
    if (total-topItem_<visible)
    {
        topItem_=std::max(0, total-visible);
        if (redraw==ro)
            drawProxy();
    }
}

void ExtendedList::drawItemBackground(Graphics& graphics, Rectangle& bounds, uint_t, bool selected)
{
    bounds.explode(0, 0, 0, -1);
    if (selected)
        drawBevel(graphics, bounds, selectedItemBackgroundColor , screenIsDoubleDensity_);
    else
        drawBevel(graphics, bounds, itemBackgroundColor, screenIsDoubleDensity_);
}

void ExtendedList::drawBackground(Graphics& graphics, const Rectangle& bounds)
{
    RGBColorType oldColor;
    WinSetBackColorRGB(&listBackgroundColor, &oldColor);
    graphics.erase(bounds);
    WinSetBackColorRGB(&oldColor, 0);
}

void ExtendedList::drawScrollBar(Graphics& graphics, const Rectangle& bounds)
{
    const int width = scrollBarWidth_-1;
    const int height = scrollButtonHeight_-1;
    RGBColorType oldBgColor;
    WinSetBackColorRGB(&itemBackgroundColor, &oldBgColor);
    RGBColorType oldFgColor;
    WinSetForeColorRGB(&foregroundColor, &oldFgColor);
    Rectangle buttonBounds(bounds.x()+1, bounds.y(), width, height);
    Rectangle orig=buttonBounds;
    drawBevel(graphics, buttonBounds, itemBackgroundColor, screenIsDoubleDensity_);
    const int bmpWidth = 7;
    const int bmpHeight = 4;
    if (frmInvalidObjectId != upBitmapId_)
        graphics.drawBitmap(upBitmapId_, Point(buttonBounds.x()+(buttonBounds.width()-bmpWidth)/2, buttonBounds.y()+(buttonBounds.height()-bmpHeight)/2));
    buttonBounds=orig;
    buttonBounds.y()+=(bounds.height()-height);
    drawBevel(graphics, buttonBounds, itemBackgroundColor, screenIsDoubleDensity_);
    if (frmInvalidObjectId != downBitmapId_)
        graphics.drawBitmap(downBitmapId_, Point(buttonBounds.x()+(buttonBounds.width()-bmpWidth)/2, buttonBounds.y()+(buttonBounds.height()-bmpHeight)/2));
    buttonBounds=orig;
    buttonBounds.y()=bounds.y()+scrollButtonHeight_;
    buttonBounds.height()=bounds.height()-2*scrollButtonHeight_;
    long totalHeight=buttonBounds.height();
    long viewCapacity=this->height()/itemHeight_;
    long itemsCount=this->itemsCount();
    assert(itemsCount>viewCapacity);
    long traktorHeight=(viewCapacity*totalHeight)/itemsCount + 1;
    traktorHeight=std::max(traktorHeight, 5L);
    graphics.erase(buttonBounds);
    WinSetBackColorRGB(&oldBgColor, NULL);
    WinSetForeColorRGB(&oldFgColor, NULL);
    buttonBounds.y()+=(long(topItem_)*totalHeight)/itemsCount;
    buttonBounds.height()=traktorHeight;
    drawBevel(graphics, buttonBounds, selectedItemBackgroundColor, screenIsDoubleDensity_);
}

ExtendedList::ItemRenderer::ItemRenderer()
{}

ExtendedList::ItemRenderer::~ItemRenderer()
{}

bool ExtendedList::handleKeyDownEvent(const EventType& event, uint_t options)
{
    assert(valid());
    if (form()->application().runningOnTreo600() && !hasFocus())
        return false;
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
    else if (((optionFireListSelectOnCenter & options) && form.fiveWayCenterPressed(&event)) ||
        chrLineFeed == event.data.keyDown.chr || chrCarriageReturn == event.data.keyDown.chr)
    {
        if (noListSelection != selection_)
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
    const int sel = selection();
    if (noListSelection == sel || (0 > delta && 0 == sel) || (0 < delta && sel == itemsCount() - 1))
        delta = 0;        
    if (0 != delta)
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
    setSelection(sel, ro);
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
        
        case nilEvent:
            handleNilEvent();
            break;
            
        default:
            handled=FormGadget::handleEvent(event);            
    }
    return handled;
}

bool ExtendedList::handleEnter(const EventType& event)
{
    assert(!trackingScrollbar_);
    focus();
    Point penPos(event.screenX, event.screenY);
    Rectangle bounds;
    this->bounds(bounds);
    assert(bounds && penPos);
    Rectangle itemsBounds=bounds;
    itemsBounds.width()-=visibleScrollBarWidth();
    if (itemsBounds && penPos)
        handlePenInItemsList(bounds, penPos, false);
    else 
        handlePenInScrollBar(bounds, penPos, false, true);
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
    if (selection_ != item && item < itemsCount)
        setSelection(item, redraw);
    if (penUp && item < itemsCount)
        fireItemSelectEvent();
}

void ExtendedList::handlePenInScrollBar(const Rectangle& bounds, const Point& penPos, bool penUp, bool enter)
{
    int viewCapacity=bounds.height()/itemHeight_;
    int itemsCount=this->itemsCount();
    int height=penPos.y-bounds.y();
    Rectangle scrollBar(bounds.x()+bounds.width()-scrollBarWidth_, bounds.y(), scrollBarWidth_, bounds.height());
    if (enter && height>=scrollButtonHeight_ && height<=scrollBar.height()-scrollButtonHeight_)
    {
        trackingScrollbar_=true;
        topItemBeforeTracking_=topItem_;
    }
    if (trackingScrollbar_)
    {
        Rectangle margin(scrollBar.x(), scrollBar.y()+scrollButtonHeight_, scrollBar.width(), scrollBar.height()-2*scrollButtonHeight_);
        margin.explode(-10, -10, 20, 20);
        int prevTopItem=topItem_;
//        if ((scrollBar && penPos) && height>=scrollButtonHeight_ && height<=scrollBar.height()-scrollButtonHeight_)
        if (margin && penPos)
        {
            long traktorTrackHeight=bounds.height()-(2*scrollButtonHeight_);
            long traktorPos=height-scrollButtonHeight_;
            long traktorHeight=(viewCapacity*traktorTrackHeight)/itemsCount;
            long newTopItem=((traktorPos-traktorHeight/2)*long(itemsCount-viewCapacity))/(traktorTrackHeight-traktorHeight);
            newTopItem=std::min(std::max(0L, newTopItem), long(itemsCount-viewCapacity));
            topItem_=newTopItem;
        }
        else
            topItem_=topItemBeforeTracking_;
        if (topItem_!=prevTopItem)
            drawProxy();
    }
    if (!penUp)
        return;
    if (trackingScrollbar_)
    {
        trackingScrollbar_=false;
        return;
    }
    if (!(scrollBar && penPos))
        return;
    if (height<scrollButtonHeight_ && 0<topItem_) 
    {
        topItem_-=viewCapacity;
        topItem_=std::max(0, topItem_);
        drawProxy();
    }
    else if (height>bounds.height()-scrollButtonHeight_ && itemsCount>viewCapacity && topItem_<itemsCount-viewCapacity)
    {
        topItem_+=viewCapacity;
        topItem_=std::min(itemsCount-viewCapacity, topItem_);
        drawProxy();
    }
}

void ExtendedList::handlePenUp(const EventType& event)
{
    Point penPos(event.screenX, event.screenY);
    Rectangle bounds;
    this->bounds(bounds);
    int visW=visibleScrollBarWidth();
    Rectangle scrollBounds(bounds.x()+bounds.width()-visW, bounds.y(), visW, bounds.height());
    if (trackingScrollbar_ || (penPos && scrollBounds))
        handlePenInScrollBar(bounds, penPos, true, false);
    else
    {
        scheduledScrollDirection_ = scheduledScrollAbandoned;
        Rectangle itemsBounds=bounds;
        itemsBounds.width() -= visW;
        itemsBounds.height() -= (itemsBounds.height() % itemHeight_);
        if (itemsBounds && penPos)
            handlePenInItemsList(bounds, penPos, true);
        else if (noListSelection != selection())
            setSelection(noListSelection, redraw);
    }
}

void ExtendedList::handlePenMove(const EventType& event)
{
    Point penPos(event.screenX, event.screenY);
    Rectangle bounds;
    this->bounds(bounds);
    int visW=visibleScrollBarWidth();
    Rectangle scrollBounds(bounds.x()+bounds.width()-visW, bounds.y(), visW, bounds.height());
    if (trackingScrollbar_)
        handlePenInScrollBar(bounds, penPos, false, false);
    else 
    {
        Rectangle itemsBounds=bounds;
        itemsBounds.width() -= visW;
        itemsBounds.height() -= (itemsBounds.height() % itemHeight_);
        if (itemsBounds && penPos)
        {
//            log().debug()<<"handlePenMove() disabling scrolling";
            scheduledScrollDirection_ = scheduledScrollAbandoned;
            handlePenInItemsList(bounds, penPos, false);
        }
        else
        {
            if (noListSelection != selection())
                setSelection(noListSelection, redraw);
            if (penPos.x >= itemsBounds.x() && penPos.x < itemsBounds.x() + itemsBounds.width())
            {
                UInt32 time = TimGetTicks();
                if (scheduledScrollAbandoned == scheduledScrollDirection_)
                {
                    if (penPos.y < bounds.y())
                        scheduledScrollDirection_ = scheduledScrollUp;
                    else
                        scheduledScrollDirection_ = scheduledScrollDown;
                    scheduledNilEventTicks_ = time + form()->application().ticksPerSecond()/7;
//                    log().debug()<<"handlePenMove() scheduling scrolling at time: "<<scheduledNilEventTicks_;
                    EvtSetNullEventTick(scheduledNilEventTicks_);
                }
                else if (time == scheduledNilEventTicks_)
                    EvtSetNullEventTick(time + 1);
            }
            else
            {
//                log().debug()<<"handlePenMove() disabling scrolling";
                scheduledScrollDirection_ = scheduledScrollAbandoned;
            }
        }
    }    
}

void ExtendedList::handleNilEvent()
{
//    log().debug()<<"handleNilEvent() enter";
    if (scheduledScrollAbandoned == scheduledScrollDirection_)
    {
//        log().debug()<<"handleNilEvent() scheduledScrollAbandoned";
        return;
    }
    UInt32 time = TimGetTicks();
//    log().debug()<<"handleNilEvent() time: "<<time;
    if (time < scheduledNilEventTicks_)
    {
//        log().debug()<<"handleNilEvent() time < scheduledNilEventTicks_";
        EvtSetNullEventTick(scheduledNilEventTicks_);
        return;
    }
    WinDirectionType dir = winUp;
    if (scheduledScrollDown == scheduledScrollDirection_)
        dir = winDown;
    scheduledNilEventTicks_ = time + form()->application().ticksPerSecond()/7;
//    log().debug()<<"handleNilEvent() scheduling scrolling at time: "<<scheduledNilEventTicks_;
    EvtSetNullEventTick(scheduledNilEventTicks_);
    scroll(dir, 1);  
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

BasicStringItemRenderer::BasicStringItemRenderer()
{}

BasicStringItemRenderer::~BasicStringItemRenderer()
{}

void BasicStringItemRenderer::drawItem(Graphics& graphics, ExtendedList& list, uint_t item, const Rectangle& itemBounds)
{
    assert(item<itemsCount());
    String text;
    getItem(text, item);
    uint_t length=text.length();
    uint_t boundsDx = itemBounds.dx()-2;
    graphics.charsInWidth(text.c_str(), length, boundsDx);
    Point p=itemBounds.topLeft;
    p.x += 2;

    uint_t boundsDy = itemBounds.dy();
    uint_t fontDy = graphics.fontHeight();

    p.y = itemBounds.y();

    if (boundsDy>fontDy)
    {
        p.y += (boundsDy-fontDy)/2;
    }
    else
    {
        p.y -= (fontDy-boundsDy)/2;
    }
    graphics.drawText(text.c_str(), length, p);
}

void ExtendedList::notifyItemsChanged()
{
    uint_t itemsCount = this->itemsCount();
    if (0 != itemsCount)
    {
        if (noListSelection == topItem_ || topItem_ >= itemsCount)
            topItem_=0;
        if (noListSelection != selection_ && selection_ >= itemsCount)
            selection_ = noListSelection;
    }            
    else 
    {
        topItem_=noListSelection;
        selection_=noListSelection;    
    }
}

bool ExtendedList::scroll(WinDirectionType direction, uint_t items)
{
    if (0 == items)
        return false;
    uint_t itemsCount = this->itemsCount();
    if (0 == itemsCount)
        return false;
    uint_t topItem = this->topItem();
    assert(topItem < itemsCount);
    if (direction == winUp)
    {
        if (0 == topItem)
            return false;
        if (items > topItem)
            topItem = 0;
        else
            topItem -= items;
    }
    else 
    {
        uint_t visible =height() / itemHeight_;
        if (visible > itemsCount)
            return false;
        uint_t newTopItem = topItem + items;
        if (newTopItem + visible >= itemsCount)
            newTopItem = itemsCount - visible;
        if (newTopItem == topItem)
            return false;
        topItem = newTopItem;
    }
    setTopItem(topItem, redraw);
    return true;
}

void ExtendedList::handleFocusChange(FocusChange change)
{
    FormGadget::handleFocusChange(change);
    if (!form()->visible())
        return;
    int sel = selection();
    if (noListSelection != sel)
    {
        uint_t itemsCount = this->itemsCount();
        Rectangle listBounds;
        bounds(listBounds);
        if (noListSelection == topItem_)
            topItem_=0;
        assert(itemsCount > topItem_);
        uint_t viewCapacity=listBounds.height()/itemHeight_;
        if (sel < topItem_)
            return;
        if (sel >= topItem_ + viewCapacity)
            return;
        bool showScrollbar = (itemsCount > viewCapacity);
        Graphics graphics(form()->windowHandle());
        drawItemProxy(graphics, listBounds, sel, showScrollbar);
    }
}
