#ifndef __STRING_LIST_FORM_HPP__
#define __STRING_LIST_FORM_HPP__

#include <RichForm.hpp>

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

    void handleListSelect(const EventType& event);

    bool handleKeyPress(const EventType& event);

    int       stringCount_;
    char_t ** strList_;
    uint_t    stringListId_;
    uint_t    selectButtonId_;
    uint_t    cancelButtonId_;
    uint_t    eventToSend_;

protected:

    enum {dontSendEvent = uint_t(-1)};

    bool handleEvent(EventType& event);

    void resize(const ArsRectangle& screenBounds);

public:
    explicit StringListForm(RichApplication& app, uint_t formId, uint_t stringListId, uint_t selectButtonId, uint_t cancelButtonId, uint_t eventToSend = dontSendEvent);

    ~StringListForm();

    void SetStringList(int stringCount, char_t* strList[]);
    
    int showModalAndGetSelection();
    
};

#endif
