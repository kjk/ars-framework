#ifndef ARSLEXIS_HISTORY_SUPPORT_HPP__
#define ARSLEXIS_HISTORY_SUPPORT_HPP__

#include <PopupMenu.hpp>

class HistoryCache;

status_t FillPopupMenuModelFromHistory(const HistoryCache& cache, PopupMenuModel& model, void* data);

status_t FillPopupMenuModelFromHistory(const char_t* cacheName, PopupMenuModel& model);

class HistorySupport
{
    Form& form_;
    PopupMenu popupMenu_;
    char_t* cacheName_;
    uint_t currentHistoryIndex_;
    uint_t popupMenuId_;
    uint_t historyButtonId_;
    
    enum ActionType 
    {
        actionSelect,
        actionNewSearch
    };
    ActionType lastAction_;
    
    bool move(bool next);
    
    bool selectEntry(HistoryCache& cache, ulong_t index);
    
public:

    typedef status_t (* PopupMenuFillHandler_t)(const HistoryCache& cache, PopupMenuModel& model, void* userData);
    
    HyperlinkHandlerBase* hyperlinkHandler;
    PopupMenuFillHandler_t popupMenuFillHandler;
    void* popupMenuFillHandlerData;

    HistorySupport(Form& form);
    
    ~HistorySupport();
    
    status_t setup(const char_t* cacheName, uint_t popupMenuId, uint_t historyButtonId, HyperlinkHandlerBase* hyperlinkHandler);
    
    bool handleEventInForm(EventType& event);
    
    /**
     * return index of that entry (entryNotFound (-1) if not found)
     */
    ulong_t setEntryTitleForUrl(const char_t* title, const char_t* url);
    
    ulong_t setLastEntryTitle(const char_t* title);
    
    /**
     * fetch hyperlinkHandle with url under given index
     * return false if index not found
     */
    bool fetchHistoryEntry(ulong_t index); 
    
    void lookupFinished(bool success, const char_t* entryTitle);
    
    bool moveNext() {return move(true);}
    
    bool movePrevious() {return move(false);}
    
    bool loadLastEntry();
    
};

#endif