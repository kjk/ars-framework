#ifndef __STRING_LIST_FORM_HPP__
#define __STRING_LIST_FORM_HPP__

#include "iPediaForm.hpp"
#include <vector>

class StringListForm: public iPediaForm
{
    ArsLexis::String listPositionsString_;

    std::vector<const char*> listPositions_;

    void updateSearchResults();

    void handleControlSelect(const EventType& data);

    void setControlsState(bool enabled);

    void handleListSelect(const EventType& event);

    bool handleKeyPress(const EventType& event);

    void handleLookupFinished(const EventType& event);

protected:

    bool handleEvent(EventType& event);

    bool handleOpen();

    void resize(const ArsLexis::Rectangle& screenBounds);

    bool handleWindowEnter(const struct _WinEnterEventType& data);

    void draw(UInt16 updateCode);

public:

    explicit StringListForm(iPediaApplication& app);

    ~StringListForm();

};

#endif