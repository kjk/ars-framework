#ifndef __SEARCH_RESULTS_FORM_HPP__
#define __SEARCH_RESULTS_FORM_HPP__

#include "iPediaForm.hpp"
#include <vector>

class SearchResultsForm: public iPediaForm
{

    ArsLexis::String listPositionsString_;
    
    typedef std::vector<const char*> ListChoices_t;
    ListChoices_t listPositions_;
    
    void updateSearchResults();

    void handleControlSelect(const EventType& data);

    void setControlsState(bool enabled);
    
    void handleListSelect(const EventType& event);
    
    void refineSearch();

    bool handleKeyPress(const EventType& event);
    
protected:
    
    bool handleEvent(EventType& event);
    
    bool handleOpen();
    
    void resize(const ArsLexis::Rectangle& screenBounds);
    
    bool handleMenuCommand(UInt16 menuItem);

    bool handleWindowEnter(const struct _WinEnterEventType& data);
    
    void draw(UInt16 updateCode);
    
public:

    explicit SearchResultsForm(iPediaApplication& app);
    
    ~SearchResultsForm();
    
};

#endif