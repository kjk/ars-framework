#include <HistorySupport.hpp>
#include <HistoryCache.hpp>
#include <Text.hpp>

status_t FillPopupMenuModelFromHistory(const char_t* cacheName, PopupMenuModel& model)
{
    HistoryCache cache;
    status_t err = cache.open(cacheName);
    if (errNone != err)
        return err;
    return FillPopupMenuModelFromHistory(cache, model, NULL);
}

status_t FillPopupMenuModelFromHistory(const HistoryCache& cache, PopupMenuModel& model, void*)
{
    uint_t count = cache.entriesCount();
    PopupMenuModel::Item* items = new_nt PopupMenuModel::Item[count];
    if (NULL == items)
        return memErrNotEnoughSpace;
        
    for (uint_t i = 0; i < count; ++i)
    {
        PopupMenuModel::Item& item = items[i];
        item.active = true;
        if (NULL == (item.text = StringCopy2(cache.entryTitle(i))))
            goto OutOfMem;
            
        if (NULL == (item.hyperlink = StringCopy2(cache.entryUrl(i))))
            goto OutOfMem;
    }
    model.setItems(items, count);
    
    return errNone;    
OutOfMem:
    delete [] items;
    return memErrNotEnoughSpace;
}

HistorySupport::HistorySupport(ArsLexis::Form& form):
    form_(form),
    popupMenu_(form),
    cacheName_(NULL),
    currentHistoryIndex_(0),
    popupMenuId_(frmInvalidObjectId),
    historyButtonId_(frmInvalidObjectId),
    popupMenuFillHandler(FillPopupMenuModelFromHistory),
    popupMenuFillHandlerData(NULL),
    hyperlinkHandler(NULL)
{
}

HistorySupport::~HistorySupport()
{
    free(cacheName_);
}

status_t HistorySupport::setup(const char_t* cacheName, uint_t popupMenuId, uint_t historyButtonId, HyperlinkHandlerBase* hyperlinkHandler)
{
    free(cacheName_);
    if (NULL == (cacheName_ = StringCopy2(cacheName)))
        return memErrNotEnoughSpace;
        
    this->hyperlinkHandler = hyperlinkHandler;        
    popupMenu_.hyperlinkHandler = hyperlinkHandler;
    popupMenuId_ = popupMenuId;
    historyButtonId_ = historyButtonId;
    return errNone;
}

bool HistorySupport::handleEventInForm(EventType& event)
{
    if (NULL == cacheName_)
        return false;
        
    if (ctlSelectEvent != event.eType || historyButtonId_ != event.data.ctlSelect.controlID)
        return false;
        
    HistoryCache cache;
    status_t err = cache.open(cacheName_);
    if (errNone != err)
        return false;
        
    if (errNone != (err = popupMenuFillHandler(cache, *popupMenu_.model(), popupMenuFillHandlerData)))
        return false;
        
    cache.close();
    popupMenu_.initialSelection = currentHistoryIndex_;
        
    RectangleType rect;
    FrmGetObjectBounds(form_, FrmGetObjectIndex(form_, historyButtonId_), &rect);
    Point point(rect.topLeft.x + rect.extent.x / 2, rect.topLeft.y + rect.extent.y / 2);
    
    Int16 sel = popupMenu_.popup(popupMenuId_, point);
    if (noListSelection == sel)
        return true;
        
    currentHistoryIndex_ = sel;
    
    return true;
}

ulong_t HistorySupport::setEntryTitleForUrl(const char_t* title, const char_t* url)
{
    HistoryCache cache;
    status_t err = cache.open(cacheName_);
    if (errNone != err)
        return HistoryCache::entryNotFound;
    
    ulong_t index = cache.entryIndex(url);
    if (HistoryCache::entryNotFound == index)
        return HistoryCache::entryNotFound;
        
    cache.setEntryTitle(index, title);
    cache.close();
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

bool HistorySupport::fetchHistoryEntry(ulong_t index)
{
    HistoryCache cache;
    status_t err = cache.open(cacheName_);
    if (errNone != err)
        return false;
    
    if (index >= cache.entriesCount())
    {
        cache.close(); 
        return false;
    }
    //TODO: this is ugly!
    String url = cache.entryUrl(index);
    cache.close(); 

    Point point;
    point.x = 10;
    point.y = 10;
    hyperlinkHandler->handleHyperlink(url, &point);
    return true;
}
