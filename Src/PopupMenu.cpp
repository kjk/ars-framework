#include <PopupMenu.hpp>
#include <Graphics.hpp>
#include <Text.hpp>
#include <UTF8_Processor.hpp>

#ifdef _PALM_OS

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
        hyperlinkHandler->handleHyperlink(hyperlink, Len(hyperlink), &p);
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

#endif

uint_t PopupMenuModel::itemsCount() const
{
    return count;
}

PopupMenuModel::PopupMenuModel():
    items(NULL),
    count(0)
{
#ifdef _PALM_OS
    setRgbColor(inactiveTextColor, 64, 64, 64);
#endif 
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
#ifdef _PALM_OS
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
#endif
#ifdef _WIN32_WCE
	// TODO: implement
	return 0;
#endif   
}

#ifdef _PALM_OS
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
    ulong_t length = Len(text);
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
#endif
 
static bool extractLong(const char*& data, ulong_t& length, long& val)
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

static bool extractString(const char*& data, ulong_t& length, const char*& str, long& strLen)
{
    long len;
    if (!extractLong(data, length, len))
        return false;
    ++len;
	assert(len > 0); 
    if (length < ulong_t(len))
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

bool PopupMenuModel::itemsFromString(const char* data, ulong_t length)
{
	ulong_t i;
    ulong_t itemsCount;
    long val;
    const char* text;
    Item* newItems = NULL;
    if (!extractLong(data, length, val))
        goto Fail;
    
	assert(val >= 0);
	itemsCount = val;
	 
    newItems = new_nt Item[itemsCount];
    if (NULL == newItems)
        goto Fail;
        
    for (i = 0; i < itemsCount; ++i)
    {
        Item& item = newItems[i];
        if (!extractString(data, length, text, val))
            goto Fail;
        
        if (0 != val)
        {
            item.text = UTF8_ToNative(text, val);
            if (NULL == item.text)
                goto Fail;
        }
        
        if (!extractString(data, length, text, val))
            goto Fail;
        
        if (0 != val)
        {
            item.hyperlink = StringCopy2N(text, val);
            if (NULL == item.hyperlink)
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

static char* StrAppendLongBE(char* str, ulong_t& length, long ll)
{
    union {
        char b[4];
        long l;
    };
    char arr[4];
    l = ll;
    for (int i = 0; i < 4; ++i)
    {
#ifdef _PALM_OS
        arr[i] = b[i];        
#else
        arr[3 - i] = b[i];
#endif
    }  
	str = StrAppend(str, length, arr, 4);
	if (NULL == str)
		return NULL;
	length += 4; 
    return str;
}

static char* StrAppendLenStrBE(char* out, ulong_t& length, const char* str)
{
    long len = Len(str);
    if (NULL == (out = StrAppendLongBE(out, length, len)))
        return NULL;

    if (NULL == (out = StrAppend(out, length, str, len + 1)))
        return NULL;
    
	length += len + 1; 
    return out;
}

status_t PopupMenuHyperlinkCreate(char*& hyperlink, ulong_t& length, const char* prefix, ulong_t itemsCount)
{
	ulong_t len = Len(prefix);
	hyperlink = StringCopyN(prefix, len);
	if (NULL == hyperlink)
		goto NoMemory;

	length = len;
	hyperlink = StrAppendLongBE(hyperlink, length, itemsCount);
	if (NULL == hyperlink)
		goto NoMemory;
    return errNone;
    
NoMemory:
	free(hyperlink);
	hyperlink = NULL;
	length = 0;
	return memErrNotEnoughSpace; 
}

status_t PopupMenuHyperlinkAppendItem(char*& hyperlink, ulong_t& length, const char_t* text, const char* link, ulong_t itemFlags)
{
	char* utf = UTF8_FromNative(text);
	if (NULL == utf)
		goto NoMemory;
	
	hyperlink = StrAppendLenStrBE(hyperlink, length, utf);
	free(utf);
	if (NULL == hyperlink)
		goto NoMemory;

	hyperlink = StrAppendLenStrBE(hyperlink, length, link);
	if (NULL == hyperlink)
		goto NoMemory;

	hyperlink = StrAppendLongBE(hyperlink, length, itemFlags);        
	if (NULL == hyperlink)
		goto NoMemory;
    
    return errNone;
    
NoMemory:
	free(hyperlink);
	hyperlink = NULL;
	length = 0;
	return memErrNotEnoughSpace; 
}

#ifdef _WIN32

long PopupMenuShow(const PopupMenuModel& model, const Point& point, HWND wnd, long initSel)
{
	long sel = -1;
	HMENU menu = CreatePopupMenu();
	if (NULL == menu)
		goto Finish;
		
	MENUITEMINFO mi;
	ZeroMemory(&mi, sizeof(mi));
	mi.cbSize = sizeof(mi);

	{
		ulong_t count = model.itemsCount();
		for (ulong_t i = 0; i < count; ++i)
		{
			const PopupMenuModel::Item& item = model.items[i];
			
			//if (item.active)
			//	mi.fState = MFS_ENABLED;
			//else
			//	mi.fState = MF_GRAYED;
			//
			//if (item.bold)
			//	mi.fState |= MFS_DEFAULT;
			//
			//mi.fMask = MIIM_TYPE|MIIM_STATE|MIIM_ID;
			//if (item.separator)
			//{
			//	mi.fType = MFT_SEPARATOR;
			//	mi.dwTypeData = NULL;
			//	mi.cch = 0;
			//}
			//else
			//{
			//	mi.fType = MFT_STRING;
			//	mi.dwTypeData = item.text;
			//	mi.cch = Len(item.text);
			//}
			//mi.wID = i + 1;
			//BOOL res = InsertMenuItem(menu, i, TRUE, &mi);
			//if (!res)
			//	goto Finish;
			UINT flags = MF_BYPOSITION;
			if (item.separator)
				flags |= MF_SEPARATOR;
			else
				flags |= MF_STRING;
				
			if (item.active)
				flags |= MF_ENABLED;
			else
				flags |= MF_GRAYED;

			// This simply doesn't work			
			//if (long(i) == initSel)
			//	flags |= MF_HILITE;
			
			BOOL res = InsertMenu(menu, -1, flags, i + 1, item.text);
			if (!res)
				goto Finish;

			if (item.bold || long(i) == initSel)
			{
				mi.fMask = MIIM_STATE;
				res = GetMenuItemInfo(menu, i, TRUE, &mi);
				if (!res)
					goto Finish;

#ifndef MFS_DEFAULT
// WM 2003 doesn't support MFS_DEFAULT, in this case define it as 0 to ignore this flag
#define MFS_DEFAULT         0
#endif
				
				// Nevertheless, this doesn't seem to work even on WM 5.0
				if (item.bold)
					mi.fState |= MFS_DEFAULT;
					
				// Neither this
				if (long(i) == initSel)
					mi.fState |= MFS_HILITE;
					
				res = SetMenuItemInfo(menu, i, TRUE, &mi);
				if (!res)
					goto Finish;
			}
			
		}
		BOOL res = TrackPopupMenuEx(menu, TPM_LEFTALIGN|TPM_TOPALIGN|TPM_NONOTIFY|TPM_RETURNCMD, point.x, point.y, wnd, NULL);
		if (0 != res)
			sel = res - 1;
	}
	
Finish:	
	if (NULL != menu)
		DestroyMenu(menu);
	return sel;
}

#ifndef NDEBUG
void test_PopupMenu(HWND wnd)
{
	char* hl = NULL;
	ulong_t l = 0;
	PopupMenuHyperlinkCreate(hl, l, "popup:", 4);
	PopupMenuHyperlinkAppendItem(hl, l, _T("Item 1"), "test: 1", popupMenuItemBold);
	PopupMenuHyperlinkAppendItem(hl, l, _T(""), "", popupMenuItemSeparator);
	PopupMenuHyperlinkAppendItem(hl, l, _T("Item 2"), "test: 2", popupMenuItemInactive);
	PopupMenuHyperlinkAppendItem(hl, l, _T("Item 3"), "test: 3", 0);
	long pos = StrFind(hl, l, _T(':'));
	assert(pos != -1);
	PopupMenuModel* model = new_nt PopupMenuModel();
	assert(NULL != model);
	++pos;
	assert(model->itemsFromString(hl + pos, l - pos));
	assert(4 == model->itemsCount());
	long sel = PopupMenuShow(*model, Point(230, 360), wnd, 3);
	delete model;
}
#endif

#endif
