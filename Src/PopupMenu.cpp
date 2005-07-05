 #include <PopupMenu.hpp>
#include <Graphics.hpp>
#include <Text.hpp>
#include <DynStr.hpp>

PopupMenu::PopupMenu(Form& form):
    list(form),
    model_(new_nt PopupMenuModel),
    hyperlinkHandler(NULL),
    initialSelection(noListSelection),
    modelOwner_(ownModel)
{
}

PopupMenu::~PopupMenu()
{
    if (list.valid())
    {
        list.form()->removeObject(list.index());
        list.detach();
    }
    if (ownModel == modelOwner_)
        delete model_;
}

bool PopupMenu::handleEventInForm(EventType& event)
{
    return false;
}

Int16 PopupMenu::popup(UInt16 id, const Point& point)
{
    assert(!list.valid());

/*    
    if (list.valid())
    {
        list.form()->removeObject(list.index());
        list.detach();
    }
 */
    
    if (NULL == model_)
        return noListSelection;
        
    ArsRectangle rect;
    rect.topLeft = point;
    uint_t fontHeight;
    {
        FontID lastFont = FntGetFont();
        FntSetFont(stdFont);
        fontHeight = FntLineHeight() ;
        FntSetFont(lastFont);
    }
    rect.height() = fontHeight * model_->count + 2;
    rect.width() = model_->maxTextWidth() + 2;
    
    ArsRectangle formBounds;
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
    list.setCustomDrawHandler(model_);
    if (noListSelection != initialSelection && list.itemsCount() > initialSelection)
        list.setSelection(initialSelection);
    
    const char_t* hyperlink = NULL;
    Int16 sel;
    while (true)
    {
        sel = LstPopupList(list);
        if (noListSelection == sel)
            break;
        const PopupMenuModel::Item& item = model_->items[sel];
        if (item.active && !item.separator)
        {
            hyperlink = item.hyperlink;
            break;
        }
    }
    list.hide();

    list.form()->removeObject(list.index());
    list.detach();

    if (NULL != hyperlinkHandler && NULL != hyperlink)
    {
        Point p;
        rect.center(p);
        hyperlinkHandler->handleHyperlink(hyperlink, &p);
    }
           
    return sel;
}

void PopupMenu::setModel(PopupMenuModel* model, ModelOwnerFlag owner)
{
    assert(NULL != model);
    if (ownModel == modelOwner_)
        delete model_;
    model_ = model;
    modelOwner_ = owner;
}

uint_t PopupMenuModel::itemsCount() const
{
    return count;
}

PopupMenuModel::PopupMenuModel():
    items(NULL),
    count(0)
{
    setRgbColor(inactiveTextColor, 64, 64, 64);
}

PopupMenuModel::~PopupMenuModel()
{
    setItems(NULL, 0);
}

PopupMenuModel::Item::Item()
{
    memzero(this, sizeof(*this));
}

PopupMenuModel::Item::~Item()
{
    free(text);
    free(hyperlink);
}

uint_t PopupMenuModel::maxTextWidth() const
{
    if (0 == count)
        return 0;
    uint_t width = 0;
    FontID lastFont = FntGetFont();
    for (uint_t i = 0; i < count; ++i)
    {
        const Item& item = items[i];
        const char_t* text = item.text;
        if (NULL == text)
            continue;

        if (item.bold)
            FntSetFont(boldFont);
        else
            FntSetFont(stdFont);

        uint_t w = FntCharsWidth(text, StrLen(text)) + 2;
        if (w > width)
            width = w;
    } 
    FntSetFont(lastFont);
    return width;
}

void PopupMenuModel::drawItem(Graphics& graphics, List& list, uint_t index, const ArsRectangle& itemBounds)
{
    assert(index < count);
    const Item& item = items[index];
    int y;
    if (item.separator)
    {
        y = itemBounds.y() + itemBounds.height() / 2;
        graphics.drawLine(itemBounds.x() - 2, y, itemBounds.x() + itemBounds.width(), y);
        return;
    }
    const char_t* text = item.text;
    if (NULL == text)
        return;
    FontID lastFont = FntGetFont();
    if (item.bold)
        FntSetFont(boldFont);
    else
        FntSetFont(stdFont);
    Point p = itemBounds.topLeft;
//    ++p.x;
    uint_t width = itemBounds.width(); // - 2;
    uint_t length = StrLen(text);
    graphics.charsInWidth(text, length, width);
    
    RGBColorType oldColor;
    if (!item.active)
        WinSetTextColorRGB(&inactiveTextColor, &oldColor);
    graphics.drawText(text, length, p);
    if (!item.active)
        WinSetTextColorRGB(&oldColor, NULL);
        
    if (!item.underlined)
        return;
    
    WinSetForeColorRGB(&inactiveTextColor, &oldColor);
    y = itemBounds.y() + itemBounds.height() - 1;
    graphics.drawLine(itemBounds.x() - 2, y, itemBounds.x() + itemBounds.width(), y);            
    WinSetForeColorRGB(&oldColor, NULL);
}

static bool extractLong(const char_t*& data, long& length, long& val)
{
    if (length < 4)
        return false;
    union 
    {
        char b[4];
        long l;
    };
    for (int i = 0; i < 4; ++i)
    {
        --length;
        unsigned int chr = (unsigned char)*data++;
        if (chr > 255)
            return false;
#ifdef _PALM_OS
        b[i] = chr;        
#else
        b[3 - i] = chr;
#endif
    }    
    val = l;
    return true;
}

static bool extractString(const char_t*& data, long& length, const char_t*& str, long& strLen)
{
    long len;
    if (!extractLong(data, length, len))
        return false;
    ++len;
    if (length < len)
        return false;
    strLen = len - 1;
    str = data;
    length -= len;
    data += len;
    return true;
}

void PopupMenuModel::setItems(Item* items, uint_t itemsCount)
{
    delete [] this->items;
    this->items = items;
    this->count = itemsCount;
}

bool PopupMenuModel::itemsFromString(const char_t* data, long length)
{
    long itemsCount;
    long val;
    const char_t* text;
    Item* newItems = NULL;
    if (!extractLong(data, length, itemsCount))
        goto Fail;
    
    newItems = new_nt Item[itemsCount];
    if (NULL == newItems)
        goto Fail;
        
    for (long i = 0; i < itemsCount; ++i)
    {
        Item& item = newItems[i];
        if (!extractString(data, length, text, val))
            goto Fail;
        
        if (0 != val)
        {
            item.text = StringCopy2N(text, val);
            if (NULL == item.text)
                goto Fail;
        }
        
        if (!extractString(data, length, text, val))
            goto Fail;
        
        if (0 != val)
        {
            item.hyperlink = StringCopy2N(text, val);
            if (NULL == item.text)
                goto Fail;
        }
                    
        if (!extractLong(data, length, val))
            goto Fail;
        
        item.active = (0 == (popupMenuItemInactive & val));
        item.bold = (0 != (popupMenuItemBold & val));
        item.separator = (0 != (popupMenuItemSeparator & val));
        item.underlined = (0 != (popupMenuItemUnderlined & val));
    }
    setItems(newItems, itemsCount);
    return true;
    
Fail:
    delete [] newItems;
    return false;        
}

static DynStr* DynStrAppendLongBE(DynStr* str, long ll)
{
    union {
        char b[4];
        long l;
    };
    char_t arr[4];
    l = ll;
    for (int i = 0; i < 4; ++i)
    {
#ifdef _PALM_OS
        arr[i] = b[i];        
#else
        arr[3 - i] = b[i]
#endif
    }  
    return DynStrAppendCharPBuf(str, arr, 4);
}

static DynStr* DynStrAppendLenStrBE(DynStr* out, const char_t* str)
{
    long len = tstrlen(str);
    if (NULL == DynStrAppendLongBE(out, len))
        return NULL;
        
    if (NULL == DynStrAppendCharPBuf(out, str, len + 1))
        return NULL;
    
    return out;
}

status_t PopupMenuHyperlinkCreate(DynStrTag* hyperlink, const char_t* prefix, ulong_t itemsCount)
{
    if (NULL == DynStrAppendCharP(hyperlink, prefix))
        return memErrNotEnoughSpace;
    
    if (NULL == DynStrAppendLongBE(hyperlink, itemsCount))
        return memErrNotEnoughSpace;
    
    return errNone;
}

status_t PopupMenuHyperlinkAppendItem(DynStrTag* hyperlink, const char_t* text, const char_t* link, ulong_t itemFlags)
{
    if (NULL == DynStrAppendLenStrBE(hyperlink, text))
        return memErrNotEnoughSpace;
        
    if (NULL == DynStrAppendLenStrBE(hyperlink, link))
        return memErrNotEnoughSpace;
        
    if (NULL == DynStrAppendLongBE(hyperlink, itemFlags))
        return memErrNotEnoughSpace;
    
    return errNone;
}


