#include <HmStyleList.hpp>
#include <Graphics.hpp>

using namespace ArsLexis;

HmStyleList::HmStyleList(Form& form, UInt16 id):
    ExtendedList(form, id)
{
    setRgbColor(breakColor_, 128, 128, 128);
    RGBColorType white = {0, 255, 255, 255};
    setListBackground(white);
    setItemBackground(white);
    RGBColorType yellow = {0, 255, 255, 0};
    setSelectedItemBackground(yellow);
    setScrollBarWidth(7);
    setScrollButtonHeight(4);
}    

HmStyleList::~HmStyleList()
{}

void HmStyleList::drawItemBackground(Graphics& graphics, Rectangle& bounds, uint_t item, bool selected)
{
    const uint_t itemsCount = this->itemsCount();
    bool last = false;
    if (itemsCount == item + 1)
        last = true;
    graphics.erase(bounds);
    if (!last) 
    {
        RGBColorType oldColor;
        WinSetForeColorRGB(&breakColor_, &oldColor);
        Coord y = bounds.y() + bounds.height() - 1;
        graphics.drawLine(bounds.x(), y, bounds.x()+bounds.width(), y);
        WinSetForeColorRGB(&oldColor, NULL);
    }
}

void HmStyleList::drawScrollBar(Graphics& graphics, const Rectangle& bounds)
{
    RGBColorType oldBgColor;
    WinSetBackColorRGB(&listBackground(), &oldBgColor);
    RGBColorType oldFgColor;
    WinSetForeColorRGB(&foreground(), &oldFgColor);
    const int bmpWidth = 7;
    const int bmpHeight = 4;
    if (frmInvalidObjectId != upBimapId())
        graphics.drawBitmap(upBimapId(), Point(bounds.x()+(scrollBarWidth()-bmpWidth)/2, bounds.y()+(scrollButtonHeight()-bmpHeight)/2));
    if (frmInvalidObjectId != downBitmapId())
        graphics.drawBitmap(downBitmapId(), Point(bounds.x()+(scrollBarWidth()-bmpWidth)/2, bounds.y()+bounds.height()-scrollButtonHeight()+(scrollButtonHeight()-bmpHeight)/2));

/*        
    Rectangle traktorBounds = 
    buttonBounds=orig;
    buttonBounds.y()=bounds.y()+scrollButtonHeight_;
    buttonBounds.height()=bounds.height()-2*scrollButtonHeight_;
    long totalHeight=buttonBounds.height();
    long viewCapacity=this->height()/itemHeight_;
    long itemsCount=this->itemsCount();
    assert(itemsCount>viewCapacity);
    long traktorHeight=(viewCapacity*totalHeight)/itemsCount+1;
    traktorHeight=std::max(traktorHeight, 5L);
    graphics.erase(buttonBounds);
    buttonBounds.y()+=(long(topItem_)*totalHeight)/itemsCount;
    buttonBounds.height()=traktorHeight;
    drawBevel(graphics, buttonBounds, selectedItemBackground_, screenIsDoubleDensity_); */
    WinSetBackColorRGB(&oldBgColor, NULL);
    WinSetForeColorRGB(&oldFgColor, NULL);
}
        
