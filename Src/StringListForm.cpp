#include <Text.hpp>
#include "StringListForm.hpp"
#include <FormObject.hpp>
#include "LookupManager.hpp"
#include "LookupHistory.hpp"
#include "MainForm.hpp"

using ArsLexis::String;
using ArsLexis::FormObject;
using ArsLexis::List;
using ArsLexis::Rectangle;
using ArsLexis::Control;
using ArsLexis::Field;
using ArsLexis::formatNumber;

void StringListForm::updateSearchResults()
{
    iPediaApplication& app = static_cast<iPediaApplication&>(application());
    LookupManager* lookupManager = app.getLookupManager();
    if (lookupManager)
    {
        listPositionsString_=lookupManager->lastExtendedSearchResults();    
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
        if (!lookupManager->lastExtendedSearchTerm().empty())
        {
            String title = lookupManager->lastExtendedSearchTerm();
            size_t resultsCount = listPositions_.size();
            char buffer[32];
            int len = formatNumber(resultsCount, buffer, sizeof(buffer));
            assert(len != -1 );
            title.append(" (");
            title.append(buffer, len);
            title.append(" results)");
            setTitle(title);
        }

    }
    {
        List list(*this, stringList);
        list.setChoices(&listPositions_[0], listPositions_.size());
    }
    update();
}

StringListForm::StringListForm(iPediaApplication& app):
    iPediaForm(app, stringListForm)
{
}

StringListForm::~StringListForm()
{
}

void StringListForm::draw(UInt16 updateCode)
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

bool StringListForm::handleOpen()
{
    bool handled=iPediaForm::handleOpen();
    updateSearchResults();
    return handled;
}

void StringListForm::resize(const ArsLexis::Rectangle& screenBounds)
{
    Rectangle rect(bounds());
    if (screenBounds!=rect)
    {
        setBounds(screenBounds);
        
        {
            List list(*this, stringList);
            list.bounds(rect);
            rect.height()=screenBounds.height()-34;
            rect.width()=screenBounds.width()-4;
            list.setBounds(rect);
            list.adjustVisibleItems();
        }
        {
            FormObject object(*this, cancelButton);
            object.bounds(rect);
            rect.y()=screenBounds.height()-14;
            rect.x()=screenBounds.width()-34;
            object.setBounds(rect);

            object.attach(selectButton);
            object.bounds(rect);
            rect.y()=screenBounds.height()-14;
            rect.x()=2;
            object.setBounds(rect);
        }        
        update();
    }
}

void StringListForm::handleControlSelect(const EventType& event)
{
    switch (event.data.ctlSelect.controlID)
    {
        case selectButton:
            break;

        case cancelButton:
        {
            MainForm* form=static_cast<MainForm*>(application().getOpenForm(mainForm));
            assert(form);
            form->update();
            closePopup();
            break;
        }
        
        default:
            assert(false);            
    }
}

void StringListForm::setControlsState(bool enabled)
{
/*    {
        Control control(*this, selectButton);
        control.setEnabled(enabled);
        control.attach(cancelButton);
        control.setEnabled(enabled);
    }
    */
}

void StringListForm::handleListSelect(const EventType& event)
{
    assert(stringList==event.data.lstSelect.listID);
    assert(listPositions_.size()>event.data.lstSelect.selection);
    iPediaApplication& app=static_cast<iPediaApplication&>(application());
    LookupManager* lookupManager=app.getLookupManager();
    if (lookupManager)
    {
        const String& term=listPositions_[event.data.lstSelect.selection];
        const LookupHistory& history=app.history();
        if (history.hasCurrentTerm() && history.currentTerm()==term)
            closePopup();
        else
            lookupManager->lookupTerm(term);
    }        
}

bool StringListForm::handleKeyPress(const EventType& event)
{
    bool handled=false;
    List list(*this, stringList);

    if (fiveWayLeftPressed(&event))
    {
        list.setSelectionDelta(-list.visibleItemsCount());
        handled = true;
    } 
    else if (fiveWayRightPressed(&event))
    {
        list.setSelectionDelta(list.visibleItemsCount());
        handled = true;
    } 
    else if (fiveWayUpPressed(&event))
    {
        list.setSelectionDelta(-1);
        handled = true;
    } 
    else if (fiveWayDownPressed(&event))
    {
        list.setSelectionDelta(1);
        handled = true;
    }
    else
    {
        switch (event.data.keyDown.chr)
        {
            case chrPageDown:
                list.setSelectionDelta(list.visibleItemsCount());
                handled=true;
                break;
                
            case chrPageUp:
                list.setSelectionDelta(-list.visibleItemsCount());
                handled=true;
                break;
            
            case chrDownArrow:
                list.scroll(winDown, 1);
                handled=true;
                break;

            case chrUpArrow:
                list.scroll(winUp, 1);
                handled=true;
                break;

            // if there is no text in the text field, select
            // the article
            case vchrRockerCenter:
            {
/*                Field fld(*this, refineSearchInputField);
                if (0==fld.textLength())
                {
                    iPediaApplication& app=static_cast<iPediaApplication&>(application());
                    LookupManager* lookupManager=app.getLookupManager();
                    if (lookupManager)
                    {
                        const String& title=listPositions_[list.selection()];
                        const LookupHistory& history=app.history();
                        if (history.hasCurrentTerm() && history.currentTerm()==title)
                            closePopup();
                        else
                            lookupManager->lookupTerm(title);
                    }
                }*/
                handled = true;
            }
            break;
    
            case chrLineFeed:
            case chrCarriageReturn:
                {
                    Control control(*this, selectButton);
                    control.hit();
                }                
                handled=true;
                break;
        }
    }
    return handled;
}

void StringListForm::handleLookupFinished(const EventType& event)
{
    setControlsState(true);
    const LookupFinishedEventData& data=reinterpret_cast<const LookupFinishedEventData&>(event.data);
    if (data.outcomeArticleBody==data.outcome)
    {
        MainForm* form=static_cast<MainForm*>(application().getOpenForm(mainForm));
        assert(form);
        if (form)
            form->setUpdateDefinitionOnEntry();
        closePopup();
        return;
    }
    else if (data.outcomeList==data.outcome)
        updateSearchResults();
    else
        update();
    LookupManager* lookupManager=static_cast<iPediaApplication&>(application()).getLookupManager();
    assert(lookupManager);
    lookupManager->handleLookupFinishedInForm(data);
}

bool StringListForm::handleEvent(EventType& event)
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

        case LookupManager::lookupFinishedEvent:
            handleLookupFinished(event);
            handled=true;
            break;     
            
        case LookupManager::lookupStartedEvent:
            setControlsState(false);            // No break is intentional.
            
        case LookupManager::lookupProgressEvent:
            update(redrawProgressIndicator);
            break;
    
        case keyDownEvent:
            handled=handleKeyPress(event);
            break;        
    
        default:
            handled=iPediaForm::handleEvent(event);
    }
    return handled;
}

bool StringListForm::handleWindowEnter(const struct _WinEnterEventType& data)
{
    const FormType* form=*this;
    iPediaApplication& app=static_cast<iPediaApplication&>(application());
    LookupManager* lookupManager=app.getLookupManager();
    if (lookupManager)
        setControlsState(!lookupManager->lookupInProgress());

    return iPediaForm::handleWindowEnter(data);
}
