#include <SysUtils.hpp>
#include <Text.hpp>
#include <StringListForm.hpp>
#include <FormObject.hpp>

StringListForm::StringListForm(RichApplication& app, uint_t formId, uint_t stringListId, uint_t selectButtonId, uint_t cancelButtonId, uint_t eventToSend):
    RichForm(app, formId, false),
    stringCount_(0),
    strList_(NULL),
    stringListId_(stringListId),
    selectButtonId_(selectButtonId),
    cancelButtonId_(cancelButtonId),
    eventToSend_(eventToSend)
{
}

StringListForm::~StringListForm()
{
}

void StringListForm::SetStringList(int stringCount, char_t *strList[])
{
    assert(stringCount>0);

    stringCount_ = stringCount;
    strList_ = strList;
}

void StringListForm::resize(const Rectangle& screenBounds)
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

void StringListForm::handleListSelect(const EventType& event)
{
    Control selButton(*this, selectButtonId_);
    selButton.hit();        
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

bool StringListForm::handleEvent(EventType& event)
{
    bool handled = false;
    switch (event.eType)
    {
            
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

int StringListForm::showModalAndGetSelection()
{
    List list(*this, stringListId_);
    list.setChoices(strList_, stringCount_);
    uint_t controlId = showModal();
    if (cancelButtonId_ == controlId)
        return NOT_SELECTED;
    else 
    {
        assert(selectButtonId_ == controlId);
        int sel = list.selection();
        if (noListSelection == sel)
            return NOT_SELECTED;
        else
            return sel;
    }
}
    
