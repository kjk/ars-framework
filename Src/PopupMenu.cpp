#include <PopupMenu.hpp>
#include <Graphics.hpp>

using namespace ArsLexis;

PopupMenu::PopupMenu(ArsLexis::Form& form):
    list(form)
{
}

PopupMenu::~PopupMenu()
{
    if (list.valid())
    {
        list.form()->removeObject(list.index());
        list.detach();
    }
}

bool PopupMenu::handleEventInForm(EventType& event)
{
    return false;
}

Int16 PopupMenu::popup(UInt16 id, const ArsLexis::Point& point)
{
    if (list.valid())
    {
        list.form()->removeObject(list.index());
        list.detach();
    }
    Rectangle rect;
    rect.topLeft = point;
    uint_t fontHeight;
    {
        FontID lastFont = FntGetFont();
        FntSetFont(stdFont);
        fontHeight = FntLineHeight() ;
        FntSetFont(lastFont);
    }
    rect.height() = fontHeight * itemDrawHandler.count + 2;
    rect.width() = itemDrawHandler.maxTextWidth() + 2;
    
    Rectangle formBounds;
    list.form()->bounds(formBounds);
    
    int maxHeight = formBounds.height() - 20;
    int maxWidth = formBounds.width() - 20;
    if (rect.height() > maxHeight)
        rect.height() = maxHeight;

    if (rect.width() > maxWidth)
        rect.width() = maxWidth;
        
    if (rect.x() + rect.width() > formBounds.width())
        rect.x() = formBounds.width() - (rect.width() + 10);
    
    if (rect.y() + rect.height() > formBounds.height())
        rect.y() = formBounds.height() - (rect.height() + 10);
    
    rect.x() -= formBounds.x();
    rect.y() -= formBounds.y();
    
    UInt16 index = list.form()->createList(id, rect, stdFont, rect.height() / fontHeight);
    if (frmInvalidObjectId == index)
        return noListSelection;
        
    list.attachByIndex(index);
    list.setCustomDrawHandler(&itemDrawHandler);
    Int16 selection = LstPopupList(list);
    
    
    return selection;
}

uint_t PopupMenuItemDrawHandler::itemsCount() const
{
    return count;
}

PopupMenuItemDrawHandler::PopupMenuItemDrawHandler():
    items(NULL),
    count(0)
{
    setRgbColor(inactiveTextColor, 64, 64, 64);
}

PopupMenuItemDrawHandler::~PopupMenuItemDrawHandler()
{
    delete [] items;
}

PopupMenuItemDrawHandler::Item::Item():
    text(NULL),
    hyperlink(NULL),
    active(false),
    bold(false)
{
}

PopupMenuItemDrawHandler::Item::~Item()
{
    free(text);
    free(hyperlink);
}

UInt16 PopupMenuItemDrawHandler::maxTextWidth() const
{
    if (0 == count)
        return 0;
    UInt16 width = 0;
    FontID lastFont = FntGetFont();
    for (UInt16 i = 0; i < count; ++i)
    {
        const Item& item = items[i];
        if (item.bold)
            FntSetFont(boldFont);
        else
            FntSetFont(stdFont);
        const char_t* text = item.text;
        if (NULL == text)
            continue;
        UInt16 w = FntCharsWidth(text, StrLen(text)) + 2;
        if (w > width)
            width = w;
    } 
    FntSetFont(lastFont);
    return width;
}

void PopupMenuItemDrawHandler::drawItem(Graphics& graphics, List& list, uint_t index, const Rectangle& itemBounds)
{
    assert(index < count);
    const Item& item = items[index];
    const char_t* text = item.text;
    if (NULL == text)
        return;
    FontID lastFont = FntGetFont();
    if (item.bold)
        FntSetFont(boldFont);
    else
        FntSetFont(stdFont);
    Point p = itemBounds.topLeft;
    ++p.x;
    uint_t width = itemBounds.width() - 2;
    uint_t length = StrLen(text);
    graphics.charsInWidth(text, length, width);
    
    RGBColorType oldTextColor;
    if (!item.active)
        WinSetTextColorRGB(&inactiveTextColor, &oldTextColor);
    graphics.drawText(text, length, p);
    if (!item.active)
        WinSetTextColorRGB(&oldTextColor, NULL);
}
