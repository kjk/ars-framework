#include "SearchResultsForm.hpp"
#include "FormObject.hpp"
#include "LookupManager.hpp"

using ArsLexis::String;
using ArsLexis::FormObject;
using ArsLexis::List;
using ArsLexis::Rectangle;
using ArsLexis::Control;
using ArsLexis::Field;

void SearchResultsForm::updateSearchResults()
{
    iPediaApplication& app=static_cast<iPediaApplication&>(application());
    LookupManager* lookupManager=app.getLookupManager();
    if (lookupManager)
    {
        listPositionsString_=lookupManager->lastSearchResults();    
        listPositions_.clear();
        String::iterator end(listPositionsString_.end());
        String::iterator lastStart=listPositionsString_.begin();
        for (String::iterator it=listPositionsString_.begin(); it!=end; ++it)
        {
            if ('\n'==*it)
            {
                const char* start=&(*lastStart);
                lastStart=it;
                ++lastStart;
                *it=chrNull;
                listPositions_.push_back(start);
            }
        }
        if (!lookupManager->lastSearchExpression().empty())
            setTitle(lookupManager->lastSearchExpression());
    }
    List list(*this, searchResultsList);
    list.setChoices(&listPositions_[0], listPositions_.size());
    
}

SearchResultsForm::SearchResultsForm(iPediaApplication& app):
    iPediaForm(app, searchResultsForm)
{
}

SearchResultsForm::~SearchResultsForm()
{
}

void SearchResultsForm::draw(UInt16 updateCode)
{
    ArsLexis::Graphics graphics;
    ArsLexis::Rectangle rect(bounds());
    Rectangle progressArea(rect.x(), rect.height()-16, rect.width(), 16);
    if (redrawAll==updateCode)
    {   
        if (visible())
            graphics.erase(progressArea);
        iPediaForm::draw(updateCode);
    }
    iPediaApplication& app=static_cast<iPediaApplication&>(application());
    LookupManager* lookupManager=app.getLookupManager();
    if (lookupManager && lookupManager->lookupInProgress())
        lookupManager->showProgress(graphics, progressArea);
}

bool SearchResultsForm::handleOpen()
{
    bool handled=iPediaForm::handleOpen();
    updateSearchResults();
    return handled;
}

void SearchResultsForm::resize(const ArsLexis::Rectangle& screenBounds)
{
    Rectangle rect(bounds());
    if (screenBounds!=rect)
    {
        setBounds(screenBounds);
        
        FormObject object(*this, searchResultsList);
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
}

void SearchResultsForm::refineSearch()
{
    Field field(*this, refineSearchInputField);
    const char* text=field.text();
    uint_t textLen;
    if (0==text || 0==(textLen=StrLen(text)))
        return;
    iPediaApplication& app=static_cast<iPediaApplication&>(application());
    LookupManager* lookupManager=app.getLookupManager();
    if (0==lookupManager)
        return;
    
    String expression(lookupManager->lastSearchExpression());
    expression.reserve(expression.length()+textLen+1);    expression.append(1, ' ').append(text, textLen);
    lookupManager->search(expression);
}

inline void SearchResultsForm::handleControlSelect(const EventType& event)
{
    switch (event.data.ctlSelect.controlID)
    {
        case refineSearchButton:
            refineSearch();
            break;
        
        case cancelButton:
            closePopup();
            break;
        
        default:
            assert(false);            
    }
}

void SearchResultsForm::setControlsState(bool enabled)
{
    {
        Control control(*this, refineSearchButton);
        control.setEnabled(enabled);
        control.attach(cancelButton);
        control.setEnabled(enabled);
    }
    {        
        Field field(*this, refineSearchInputField);
        if (enabled)
        {
            field.show();
            field.focus();
        }
        else
        {
            releaseFocus();
            field.hide();
        }
    }            
}

void SearchResultsForm::handleListSelect(const EventType& event)
{
    assert(searchResultsList==event.data.lstSelect.listID);
    assert(listPositions_.size()>event.data.lstSelect.selection);
    iPediaApplication& app=static_cast<iPediaApplication&>(application());
    LookupManager* lookupManager=app.getLookupManager();
    if (lookupManager)
    {
        const String& term=listPositions_[event.data.lstSelect.selection];
        if (!lookupManager->history().empty() && lookupManager->history().currentTerm()==term)
            closePopup();
        else
            lookupManager->lookupTerm(term);
    }        
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
            
        case lstSelectEvent:
            handleListSelect(event);
            handled=true;
            break;

        case iPediaApplication::appLookupFinishedEvent:
            {
                setControlsState(true);
                const LookupManager::LookupFinishedEventData& data=reinterpret_cast<const LookupManager::LookupFinishedEventData&>(event.data);
                if (data.outcomeDefinition==data.outcome)
                {
                    handled=true;
                    closePopup();
                }
                else if (data.outcomeList==data.outcome)
                    updateSearchResults();
                else
                    update();
            }
            break;     
            
        case iPediaApplication::appLookupStartedEvent:
            setControlsState(false);            // No break is intentional.
            
        case iPediaApplication::appLookupProgressEvent:
            update(redrawProgressIndicator);
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
    iPediaApplication& app=static_cast<iPediaApplication&>(application());
    LookupManager* lookupManager=app.getLookupManager();
    if (lookupManager)
        setControlsState(!lookupManager->lookupInProgress());

    return iPediaForm::handleWindowEnter(data);
}
