#ifndef ARSLEXIS_HISTORY_SUPPORT_HPP__
#define ARSLEXIS_HISTORY_SUPPORT_HPP__

#include <PopupMenu.hpp>

class HistoryCache;

status_t FillPopupMenuModelFromHistory(const HistoryCache& cache, PopupMenuModel& model, void* data);

//status_t FillPopupMenuModelFromHistory(const char_t* cacheName, PopupMenuModel& model);

class HistorySupport
{
    Form& form_;
    PopupMenu popupMenu_;
    char_t* cacheName_;
    uint_t popupMenuId_;
    uint_t historyButtonId_;
    
    enum ActionType 
    {
        actionSelect,
        actionNewSearch
    };
    ActionType lastAction_;
    
    bool selectEntry(HistoryCache& cache, ulong_t index);
    
    bool followUrl(HistoryCache& cache, const char_t* url);
    
public:

    uint_t currentHistoryIndex;

    typedef status_t (* PopupMenuFillHandler_t)(const HistoryCache& cache, PopupMenuModel& model, void* userData);
    
    PopupMenuFillHandler_t popupMenuFillHandler;
    void* popupMenuFillHandlerData;
    
    HyperlinkHandlerBase* hyperlinkHandler;
    
    typedef bool (* CacheReadHandler_t)(HistoryCache& cache, const char_t* url);
    
    CacheReadHandler_t cacheReadHandler;

    HistorySupport(Form& form);
    
    ~HistorySupport();
    
    status_t setup(const char_t* cacheName, uint_t popupMenuId, uint_t historyButtonId, HyperlinkHandlerBase* hyperlinkHandler, CacheReadHandler_t cacheReadHandler);
    
    bool handleEventInForm(EventType& event);
    
    /**
     * return index of that entry (entryNotFound if not found)
     */
    ulong_t setEntryTitleForUrl(const char_t* title, const char_t* url);
    
    ulong_t setLastEntryTitle(const char_t* title);
    
    /**
     * fetch hyperlinkHandle with url under given index
     * return false if index not found
     */
    bool fetchHistoryEntry(ulong_t index); 
    
    long lookupFinished(bool success, const char_t* entryTitle);
    
    bool move(int delta);
    
    bool moveNext() {return move(1);}
    
    bool movePrevious() {return move(-1);}
    
    bool loadLastEntry();
    
    const char_t* cacheName() const {return cacheName_;}
    
};

#endif