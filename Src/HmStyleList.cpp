#include <HmStyleList.hpp>
#include <Graphics.hpp>
#include <68k/Hs.h>

HmStyleList::HmStyleList(Form& form, UInt16 id):
    ExtendedList(form, id)
{
    setRgbColor(breakColor_, 128, 128, 128);
    
    UIColorGetTableEntryRGB(UIObjectFill, &listBackgroundColor);
    itemBackgroundColor = listBackgroundColor;
    UIColorGetTableEntryRGB(UIObjectForeground, &foregroundColor);
    UIColorGetTableEntryRGB(UIObjectSelectedForeground, &selectedForegroundColor);
    UIColorGetTableEntryRGB(UIObjectSelectedFill, &selectedItemBackgroundColor);
    noFocusItemBackgroundColor = selectedItemBackgroundColor;
    
    if (form.application().runningOnTreo600())
        HsNavGetFocusColor(hsNavFocusColorSecondaryHighlight, &noFocusItemBackgroundColor);

/*    
    RGBColorType white = {0, 255, 255, 255};
    setListBackground(white);
    setItemBackground(white);
    RGBColorType yellow = {0, 255, 255, 0};
    RGBColorType treoBlue = {0, 156, 207, 206};
    
    RGBColorType itemBgColor = treoBlue;
    setSelectedItemBackground(itemBgColor);
    
*/    
    setScrollBarWidth(7);
    setScrollButtonHeight(4);
}

HmStyleList::~HmStyleList()
{}

void HmStyleList::drawItemBackground(Graphics& graphics, ArsRectangle& bounds, uint_t item, bool selected)
{
    const uint_t itemsCount = this->itemsCount();
    graphics.erase(bounds);
    RGBColorType oldColor;
    WinSetForeColorRGB(&breakColor_, &oldColor);
    Coord y = bounds.y() + bounds.height() - 1;
    graphics.drawLine(bounds.x(), y, bounds.x()+bounds.width(), y);
    WinSetForeColorRGB(&oldColor, NULL);
}

void HmStyleList::drawScrollBar(Graphics& graphics, const ArsRectangle& bounds)
{
    RGBColorType oldBgColor;
    WinSetBackColorRGB(&listBackgroundColor, &oldBgColor);
    RGBColorType oldFgColor;
    WinSetForeColorRGB(&foregroundColor, &oldFgColor);
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
    WinSetBackColorRGB(&foregroundColor, NULL);
    WinSetForeColorRGB(&listBackgroundColor, NULL);
    WinSetPatternType(grayPattern);
    RectangleType rect = {{traktorX, boundsY + scrollButtonHeight + resMulti}, {traktorWidth, totalHeight}};
    WinPaintRectangle(&rect, 0);
    WinSetBackColorRGB(&foregroundColor, NULL);
    WinSetPatternType(blackPattern);
    graphics.erase(ArsRectangle(traktorX, traktorY, traktorWidth, traktorHeight));

    if (screenIsDoubleDensity())
        WinSetCoordinateSystem(coordSystem);
        
    WinSetBackColorRGB(&oldBgColor, NULL);
    WinSetForeColorRGB(&oldFgColor, NULL);
    WinSetPatternType(oldPattern);
}
        
