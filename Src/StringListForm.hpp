#ifndef __STRING_LIST_FORM_HPP__
#define __STRING_LIST_FORM_HPP__

#include <RichForm.hpp>

using ArsLexis::RichApplication;
using ArsLexis::RichForm;
using ArsLexis::char_t;

#define NOT_SELECTED -1

struct StringListEventData
{
    int value;
    StringListEventData(int aValue) :
        value(aValue) {}
};

class StringListForm: public RichForm
{
    void handleControlSelect(const EventType& data);

    void handleListSelect(const EventType& event);

    bool handleKeyPress(const EventType& event);

    int       stringCount_;
    char_t ** strList_;
    uint_t    stringListId_;
    uint_t    selectButtonId_;
    uint_t    cancelButtonId_;
    uint_t    eventToSend_;

protected:

    bool handleEvent(EventType& event);

    bool handleOpen();

    bool handleWindowEnter(const struct _WinEnterEventType& data);

    void resize(const ArsLexis::Rectangle& screenBounds);

public:
    explicit StringListForm(RichApplication& app, uint_t formId, uint_t stringListId, uint_t selectButtonId, uint_t cancelButtonId);

    ~StringListForm();

    void SetStringList(int stringCount, char_t *strList[], uint_t eventToSend);
};

#endif
