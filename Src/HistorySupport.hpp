#ifndef ARSLEXIS_HISTORY_SUPPORT_HPP__
#define ARSLEXIS_HISTORY_SUPPORT_HPP__

#include <PopupMenu.hpp>

class HistoryCache;

status_t FillPopupMenuModelFromHistory(const HistoryCache& cache, PopupMenuModel& model, void* data);

status_t FillPopupMenuModelFromHistory(const char_t* cacheName, PopupMenuModel& model);

class HistorySupport
{
    ArsLexis::Form& form_;
    PopupMenu popupMenu_;
    char_t* cacheName_;
    uint_t currentHistoryIndex_;
    uint_t popupMenuId_;
    uint_t historyButtonId_;
    
public:

    typedef status_t (* PopupMenuFillHandler_t)(const HistoryCache& cache, PopupMenuModel& model, void* userData);
    
    HyperlinkHandlerBase* hyperlinkHandler;
    PopupMenuFillHandler_t popupMenuFillHandler;
    void* popupMenuFillHandlerData;

    HistorySupport(ArsLexis::Form& form);
    
    ~HistorySupport();
    
    status_t setup(const char_t* cacheName, uint_t popupMenuId, uint_t historyButtonId, HyperlinkHandlerBase* hyperlinkHandler);
    
    bool handleEventInForm(EventType& event);
    
};

#endif