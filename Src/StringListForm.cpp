#include <SysUtils.hpp>
#include <Text.hpp>
#include <StringListForm.hpp>
#include <FormObject.hpp>

using ArsLexis::String;
using ArsLexis::FormObject;
using ArsLexis::List;
using ArsLexis::Rectangle;
using ArsLexis::Control;
using ArsLexis::sendEvent;

StringListForm::StringListForm(RichApplication& app, uint_t formId, uint_t stringListId, uint_t selectButtonId, uint_t cancelButtonId):
    RichForm(app, formId, false)
{
    stringCount_ = 0;
    strList_ = NULL;
    stringListId_ = stringListId;
    selectButtonId_ = selectButtonId;
    cancelButtonId_ = cancelButtonId;
}

StringListForm::~StringListForm()
{
}

void StringListForm::SetStringList(int stringCount, char_t *strList[], uint_t eventToSend)
{
    assert(stringCount>0);

    stringCount_ = stringCount;
    strList_ = strList;
    eventToSend_ = eventToSend;
}

void StringListForm::resize(const ArsLexis::Rectangle& screenBounds)
{
    Rectangle rect(bounds());
    if (screenBounds==rect)
        return;

    setBounds(screenBounds);
    
    {
        List list(*this, stringListId_);
        list.bounds(rect);
        rect.height() = screenBounds.height()-34;
        rect.width() = screenBounds.width()-4;
        list.setBounds(rect);
        list.adjustVisibleItems();
    }

    {
        FormObject object(*this, cancelButtonId_);
        object.bounds(rect);
        rect.y() = screenBounds.height()-14;
        rect.x() = screenBounds.width()-34;
        object.setBounds(rect);

        object.attach(selectButtonId_);
        object.bounds(rect);
        rect.y() = screenBounds.height()-14;
        rect.x() = 2;
        object.setBounds(rect);
    }        
    update();
}

void StringListForm::handleControlSelect(const EventType& event)
{
    uint_t controlId = event.data.ctlSelect.controlID;
    if ( controlId == selectButtonId_ )
    {
        List list(*this, stringListId_);
        closePopup();
        sendEvent(eventToSend_, StringListEventData(list.selection()));
    }
    else if ( controlId == cancelButtonId_)
    {
        closePopup();
        sendEvent(eventToSend_, StringListEventData(NOT_SELECTED));
    }
#ifdef DEBUG
    else
        assert(false);            
#endif
}

void StringListForm::handleListSelect(const EventType& event)
{
    assert(stringListId_==event.data.lstSelect.listID);
    assert(event.data.lstSelect.selection < stringCount_);

    closePopup();
    sendEvent(eventToSend_, StringListEventData(event.data.lstSelect.selection));
}

bool StringListForm::handleKeyPress(const EventType& event)
{
    bool handled = false;
    List list(*this, stringListId_);

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
                handled = true;
                break;
                
            case chrPageUp:
                list.setSelectionDelta(-list.visibleItemsCount());
                handled = true;
                break;
            
            case chrDownArrow:
                list.scroll(winDown, 1);
                handled = true;
                break;

            case chrUpArrow:
                list.scroll(winUp, 1);
                handled = true;
                break;

            // if there is no text in the text field, select
            // the article
            case vchrRockerCenter:
            {
                Control control(*this, selectButtonId_);
                control.hit();
                handled = true;
            }
            break;
    
            case chrLineFeed:
            case chrCarriageReturn:
            {
                Control control(*this, selectButtonId_);
                control.hit();
                handled = true;
            }                
            break;
        }
    }
    return handled;
}

bool StringListForm::handleOpen()
{
    bool handled = RichForm::handleOpen();
    List list(*this, stringListId_);
    list.setChoices(strList_, (uint_t)stringCount_);
    update();
    return handled;
}

bool StringListForm::handleWindowEnter(const struct _WinEnterEventType& data)
{
    const FormType* form=*this;
    if (data.enterWindow==static_cast<const void*>(form))
    {
        List list(*this, stringListId_);
        list.setChoices(strList_, (uint_t)stringCount_);
    }
    update();
    return RichForm::handleWindowEnter(data);
}

bool StringListForm::handleEvent(EventType& event)
{
    bool handled = false;
    switch (event.eType)
    {
        case ctlSelectEvent:
            handleControlSelect(event);
            handled = true;
            break;
            
        case lstSelectEvent:
            handleListSelect(event);
            handled = true;
            break;

        case keyDownEvent:
            handled = handleKeyPress(event);
            break;

        default:
            handled = RichForm::handleEvent(event);
    }
    return handled;
}

