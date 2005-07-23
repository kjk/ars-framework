#include <HistorySupport.hpp>
#include <HistoryCache.hpp>
#include <Text.hpp>

/*
status_t FillPopupMenuModelFromHistory(const char_t* cacheName, PopupMenuModel& model)
{
    HistoryCache cache;
    status_t err = cache.open(cacheName);
    if (errNone != err)
        return err;
    return FillPopupMenuModelFromHistory(cache, model, NULL);
}
*/

status_t FillPopupMenuModelFromHistory(const HistoryCache& cache, PopupMenuModel& model, void* userData)
{
	uint_t i;
    uint_t count = cache.entriesCount();
    const char* homeUrl = static_cast<const char*>(userData);
    uint_t start = 0;
    if (NULL != homeUrl)
    {
        ++count;
        start = 1;
    }
    
    PopupMenuModel::Item* items = new_nt PopupMenuModel::Item[count];
    if (NULL == items)
        return memErrNotEnoughSpace;
        
    PopupMenuModel::Item* item;
    if (NULL != homeUrl)
    {
        item = &items[0];
        item->text = StringCopy2(_T("Home"));
        if (NULL == item->text)
            goto OutOfMem;
        
        item->hyperlink = StringCopy2(homeUrl);
        if (NULL == item->hyperlink)
            goto OutOfMem;
        
        item->active = true;
        item->bold = true;
        if (1 != count)
            item->underlined = true;
    }
    for (i = start; i < count; ++i)
    {
        item = &items[i];
        item->active = true;
        if (NULL == (item->text = StringCopy2(cache.entryTitle(i - start))))
            goto OutOfMem;
            
        if (NULL == (item->hyperlink = StringCopy2(cache.entryUrl(i - start))))
            goto OutOfMem;
    }
    model.setItems(items, count);
    
    return errNone;    
OutOfMem:
    delete [] items;
    return memErrNotEnoughSpace;
}

#ifdef _PALM_OS
HistorySupport::HistorySupport(Form& form):
    form_(form),
    popupMenu_(form),
    popupMenuId_(frmInvalidObjectId),
    historyButtonId_(frmInvalidObjectId),
#endif
#ifdef _WIN32
HistorySupport::HistorySupport(HWND wnd):
	window_(wnd),
	commandId_(0),
#endif
    cacheName_(NULL),
    currentHistoryIndex(0),
    popupMenuFillHandler(FillPopupMenuModelFromHistory),
    popupMenuFillHandlerData(NULL),
    hyperlinkHandler(NULL),
    lastAction_(actionNewSearch),
    cacheReadHandler(NULL)
{
}

HistorySupport::~HistorySupport()
{
    free(cacheName_);
}

#ifdef _PALM_OS
status_t HistorySupport::setup(const char_t* cacheName, uint_t popupMenuId, uint_t historyButtonId, HyperlinkHandlerBase* hh, CacheReadHandler_t readHandler)
{
    free(cacheName_);
    if (NULL == (cacheName_ = StringCopy2(cacheName)))
        return memErrNotEnoughSpace;
        
    this->hyperlinkHandler = hh;        
    popupMenuId_ = popupMenuId;
    historyButtonId_ = historyButtonId;
    cacheReadHandler = readHandler;
    return errNone;
}
#endif

#ifdef _WIN32
status_t HistorySupport::setup(const char_t* cacheName, UINT comandId, HyperlinkHandlerBase* hh, CacheReadHandler_t readHandler)
{
    free(cacheName_);
    if (NULL == (cacheName_ = StringCopy2(cacheName)))
        return memErrNotEnoughSpace;
        
    hyperlinkHandler = hh;
	commandId_ = comandId; 
    cacheReadHandler = readHandler;
    return errNone;
}
#endif

#ifdef _PALM_OS
bool HistorySupport::handleEventInForm(EventType& event)
{
    assert(NULL != cacheReadHandler);
    if (NULL == cacheName_)
        return false;
        
    if (ctlSelectEvent != event.eType || historyButtonId_ != event.data.ctlSelect.controlID)
        return false;
        
    HistoryCache cache;
    status_t err = cache.open(cacheName_);
    if (errNone != err)
        return true;
    
    if (0 == cache.entriesCount() && NULL == popupMenuFillHandlerData)
        return true;
        
    if (errNone != (err = popupMenuFillHandler(cache, *popupMenu_.model(), popupMenuFillHandlerData)))
        return true;

    popupMenu_.initialSelection = currentHistoryIndex;
    if (NULL != popupMenuFillHandlerData)
        ++popupMenu_.initialSelection;
        
    RectangleType rect;
    FrmGetObjectBounds(form_, FrmGetObjectIndex(form_, historyButtonId_), &rect);
    Point point(rect.topLeft.x + rect.extent.x / 2, rect.topLeft.y + rect.extent.y / 2);
    
    lastAction_ = actionSelect;
    Int16 sel = popupMenu_.popup(popupMenuId_, point);
    if (noListSelection == sel)
    {
        lastAction_ = actionNewSearch;
        return true;
    }
    if (0 == sel && NULL != popupMenuFillHandlerData)
        lastAction_ = actionNewSearch;
    
    const char_t* url = static_cast<const char_t*>(popupMenuFillHandlerData);
    if (NULL == url || sel > 0)
        currentHistoryIndex = sel;
    if (NULL != url)
    {
        if (0 == sel)
        {
            cache.close();
            if (NULL == hyperlinkHandler)
                return true;
            hyperlinkHandler->handleHyperlink(url, NULL);
            return true;
        }
        else
            --currentHistoryIndex;
    }
    url = cache.entryUrl(currentHistoryIndex);
    followUrl(cache, url);
    return true;
}
#endif

#ifdef _WIN32
LRESULT HistorySupport::handleEventInForm(UINT msg, WPARAM wParam)
{
    assert(NULL != cacheReadHandler);
    if (NULL == cacheName_)
        return TRUE;
    
	if (WM_COMMAND != msg || LOWORD(wParam) != commandId_)
		return TRUE;
	
    HistoryCache cache;
    status_t err = cache.open(cacheName_);
    if (errNone != err)
        return FALSE;
    
    if (0 == cache.entriesCount() && NULL == popupMenuFillHandlerData)
        return FALSE;
    
	PopupMenuModel model; 
    if (errNone != (err = popupMenuFillHandler(cache, model, popupMenuFillHandlerData)))
        return FALSE;

	ulong_t initialSelection = currentHistoryIndex;
    if (NULL != popupMenuFillHandlerData)
        ++initialSelection;
	
	HWND control = GetDlgItem(window_, commandId_);
	assert(NULL != control);

	RECT rect;
	GetWindowRect(control, &rect);
	Point p((rect.bottom - rect.top)/2, (rect.right - rect.left)/2);
	
	long sel = PopupMenuShow(model, p, window_, initialSelection);
    if (-1 == sel)
    {
        lastAction_ = actionNewSearch;
        return FALSE;
    }
    if (0 == sel && NULL != popupMenuFillHandlerData)
        lastAction_ = actionNewSearch;
    
    const char* url = static_cast<const char*>(popupMenuFillHandlerData);
    if (NULL == url || sel > 0)
        currentHistoryIndex = sel;
        
    if (NULL != url)
    {
        if (0 == sel)
        {
            cache.close();
            if (NULL == hyperlinkHandler)
                return FALSE;
                
            hyperlinkHandler->handleHyperlink(url, Len(url), NULL);
            return FALSE;
        }
        else
            --currentHistoryIndex;
    }
    url = cache.entryUrl(currentHistoryIndex);
    followUrl(cache, url);
    return FALSE;
}
#endif

ulong_t HistorySupport::setEntryTitleForUrl(const char_t* title, const char* url)
{
    HistoryCache cache;
    status_t err = cache.open(cacheName_);
    if (errNone != err)
        return HistoryCache::entryNotFound;
    
    ulong_t index = cache.entryIndex(url);
    if (HistoryCache::entryNotFound == index)
        return HistoryCache::entryNotFound;
        
    cache.setEntryTitle(index, title);
    return index;
}

ulong_t HistorySupport::setLastEntryTitle(const char_t* title)
{
    HistoryCache cache;
    status_t err = cache.open(cacheName_);
    if (errNone != err)
        return HistoryCache::entryNotFound;        

    if (0 == cache.entriesCount())
        return HistoryCache::entryNotFound;
        
    cache.setEntryTitle(cache.entriesCount() - 1, title);
    return cache.entriesCount() - 1;
}

bool HistorySupport::selectEntry(HistoryCache& cache, ulong_t index)
{
    if (index >= cache.entriesCount())
        return false;
    
    const char* url = cache.entryUrl(index);
    currentHistoryIndex = index;
    lastAction_ = actionSelect;
    
    return followUrl(cache, url);
}


bool HistorySupport::fetchHistoryEntry(ulong_t index)
{
    HistoryCache cache;
    status_t err = cache.open(cacheName_);
    if (errNone != err)
        return false;

    return selectEntry(cache, index);    
}

long HistorySupport::lookupFinished(bool success, const char_t* entryTitle)
{
    
    if (!success)
    {
        lastAction_ = actionNewSearch;    
        return -1;
    }
        
    HistoryCache cache;
    status_t err = cache.open(cacheName_);
    if (errNone != err)
        return -1;
    
    if (actionNewSearch == lastAction_)
    {
        assert(0 != cache.entriesCount());
        if (0 == cache.entriesCount())
            return -1;
            
        currentHistoryIndex = cache.entriesCount() - 1;
        cache.setEntryTitle(currentHistoryIndex, entryTitle);

        // Special history flags 'h' and 'H'
        // TODO: write this better!
        char* url = (char*) cache.entryUrl(currentHistoryIndex);
        bool wasSpecialHistory = false;
        // TODO: change 'h' to 'H'
        if (Len(url) > 2)
            if ('h' == url[0] && 's' == url[1] && '+' == url[2])
            {
                url[0] = 'H';
                wasSpecialHistory = true;
            }    

        // TODO: remove all other entries with this url
        if (wasSpecialHistory)
        {
            for (uint_t i = cache.entriesCount() - 1; i >= 0; i--)
            {
                if (i != currentHistoryIndex)
                    if (StrEquals(url, cache.entryUrl(i)))
                    {
                        if (i > currentHistoryIndex)
                            cache.removeEntry(i);
                        else
                        {
                            cache.removeEntry(i);
                            currentHistoryIndex--;
                            url = (char*) cache.entryUrl(currentHistoryIndex);
                        }
                    }
            }        
        }      
        // end of special history flags
    }
    lastAction_ = actionNewSearch;
    return currentHistoryIndex;
}

bool HistorySupport::move(int delta)
{
    if (0 == delta)
        return true;
        
    long index = long(currentHistoryIndex) + delta;
    if (index < 0)
        return false;
        
    HistoryCache cache;
    status_t err = cache.open(cacheName_);
    if (errNone != err)
        return false;
        
    return selectEntry(cache, index);    
}

bool HistorySupport::loadLastEntry()
{
    HistoryCache cache;
    status_t err = cache.open(cacheName_);
    if (errNone != err)
        return false;
    
    if (0 == cache.entriesCount())
        return false;
        
    return selectEntry(cache, cache.entriesCount() - 1);    
}

bool HistorySupport::followUrl(HistoryCache& cache, const char* url)
{
    assert(NULL != cacheReadHandler);
    if (cacheReadHandler(cache, url))
        return true;
    if (NULL == hyperlinkHandler)
        return false;
    
    cache.close();

    hyperlinkHandler->handleHyperlink(url, Len(url), NULL);
	return true;
}


