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
    long viewCapacity=this->height()/itemHeight();
    long itemsCount=this->itemsCount();
    assert(itemsCount>viewCapacity);

    int resMulti = 1;
    UInt16 coordSystem;
    if (screenIsDoubleDensity())
    {
        resMulti = 2;
        coordSystem = WinSetCoordinateSystem(kCoordinatesNative);
    }

    int traktorWidth = 3*resMulti;
    int scrollBarWidth = resMulti * this->scrollBarWidth();
    int scrollButtonHeight = resMulti * this->scrollButtonHeight();
    long totalHeight = (bounds.height()-2)*resMulti  - scrollButtonHeight*2;
    long traktorHeight = (viewCapacity*totalHeight)/itemsCount + 1;
    long minTraktorHeight = 5*resMulti;
    int boundsX = bounds.x()*resMulti;
    int boundsY = bounds.y()*resMulti;
    int traktorX = boundsX + (scrollBarWidth - traktorWidth)/2;
    int traktorY = resMulti + boundsY + scrollButtonHeight + (long(topItem())*totalHeight)/itemsCount;
    
    traktorHeight = std::max(traktorHeight, minTraktorHeight);

    PatternType oldPattern = WinGetPatternType();
    WinSetBackColorRGB(&foreground(), NULL);
    WinSetForeColorRGB(&listBackground(), NULL);
    WinSetPatternType(grayPattern);
    RectangleType rect = {{traktorX, boundsY + scrollButtonHeight + resMulti}, {traktorWidth, totalHeight}};
    WinPaintRectangle(&rect, 0);
    WinSetBackColorRGB(&foreground(), NULL);
    WinSetPatternType(blackPattern);
    graphics.erase(Rectangle(traktorX, traktorY, traktorWidth, traktorHeight));

    if (screenIsDoubleDensity())
        WinSetCoordinateSystem(coordSystem);
        
    WinSetBackColorRGB(&oldBgColor, NULL);
    WinSetForeColorRGB(&oldFgColor, NULL);
    WinSetPatternType(oldPattern);
}
        
