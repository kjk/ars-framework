#include "SearchResultsForm.hpp"
#include "FormObject.hpp"

using ArsLexis::String;
using ArsLexis::FormObject;
using ArsLexis::List;
using ArsLexis::Rectangle;

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
    Rectangle rect(2, 2, screenBounds.width()-4, screenBounds.height()-4);
    setBounds(rect);
    
    FormObject object(*this, searchResultsList);
    object.bounds(rect);
    rect.height()=screenBounds.height()-36;
    rect.width()=screenBounds.width()-8;
    object.setBounds(rect);

    object.attach(cancelButton);
    object.bounds(rect);
    rect.y()=screenBounds.height()-18;
    object.setBounds(rect);
    
    update();
}

inline void SearchResultsForm::handleControlSelect(const ctlSelect& data)
{
    closePopup();
}


bool SearchResultsForm::handleEvent(EventType& event)
{
    bool handled=false;
    switch (event.eType)
    {
        case ctlSelectEvent:
            handleControlSelect(event.data.ctlSelect);
            handled=true;
            break;
    
        default:
            handled=iPediaForm::handleEvent(event);
    }
    return handled;
}

