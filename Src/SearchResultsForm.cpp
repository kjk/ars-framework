#include "SearchResultsForm.hpp"
#include "FormObject.hpp"

using ArsLexis::String;
using ArsLexis::FormObject;
using ArsLexis::List;
using ArsLexis::Rectangle;
using ArsLexis::Control;

void SearchResultsForm::prepareListChoices()
{
    String::iterator end(listPositionsString_.end());
    String::iterator lastStart=listPositionsString_.begin();
    for (String::iterator it=listPositionsString_.begin(); it!=end; ++it)
    {
        if ('\n'==*it)
        {
            ++listPositionsCount_;
            const char* start=&(*lastStart);
            lastStart=it;
            ++lastStart;
            *it=chrNull;
            listPositions_.push_back(start);
        }
    }
}

SearchResultsForm::SearchResultsForm(iPediaApplication& app):
    iPediaForm(app, searchResultsForm),
    listPositionsString_(app.searchResults()),
    listPositionsCount_(0)
{
    listPositionsString_="Test\nWest\nRest\nBrest\n";
    prepareListChoices();
}

SearchResultsForm::~SearchResultsForm()
{
}

bool SearchResultsForm::handleOpen()
{
    bool handled=iPediaForm::handleOpen();
    if (!listPositions_.empty())
    {
        List list(*this, searchResultsList);
        list.setChoices(&listPositions_[0], listPositions_.size());
    }
    return handled;
}

void SearchResultsForm::resize(const ArsLexis::Rectangle& screenBounds)
{
    setBounds(screenBounds);
    
    FormObject object(*this, searchResultsList);
    Rectangle rect;
    object.bounds(rect);
    rect.height()=screenBounds.height()-34;
    rect.width()=screenBounds.width()-4;
    object.setBounds(rect);

    object.attach(refineSearchInputField);
    object.bounds(rect);
    rect.y()=screenBounds.height()-14;
    rect.width()=screenBounds.width()-74;
    object.setBounds(rect);

    object.attach(cancelButton);
    object.bounds(rect);
    rect.y()=screenBounds.height()-14;
    rect.x()=screenBounds.width()-34;
    object.setBounds(rect);

    object.attach(refineSearchButton);
    object.bounds(rect);
    rect.y()=screenBounds.height()-14;
    rect.x()=screenBounds.width()-69;
    object.setBounds(rect);
    
    update();
}

inline void SearchResultsForm::handleControlSelect(const EventType& event)
{
    closePopup();
}


bool SearchResultsForm::handleEvent(EventType& event)
{
    bool handled=false;
    switch (event.eType)
    {
        case ctlSelectEvent:
            handleControlSelect(event);
            handled=true;
            break;
    
        default:
            handled=iPediaForm::handleEvent(event);
    }
    return handled;
}

bool SearchResultsForm::handleMenuCommand(UInt16 menuItem)
{
    bool handled=false;
    Control control(*this);
    switch (menuItem)
    {
        case refineSearchMenuItem:
            control.attach(refineSearchButton);
            break;
       
        case cancelSearchMenuItem:
            control.attach(cancelButton);
            break;
        
        default:
            assert(false);            
    }
    if (control.valid())
    {
        control.hit();
        handled=true;
    }
    return handled;
}

bool SearchResultsForm::handleWindowEnter(const struct _WinEnterEventType& data)
{
    const FormType* form=*this;
    if (data.enterWindow==static_cast<const void*>(form))
    {
        FormObject object(*this, refineSearchInputField);
        object.focus();
    }
    return iPediaForm::handleWindowEnter(data);
}
