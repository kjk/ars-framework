#include "iPediaForm.hpp"

Boolean iPediaForm::handleEvent(EventType& event)
{
    Boolean handled=false;
    switch (event.eType)
    {
        case winDisplayChangedEvent:
            resize(event.data.winDisplayChanged.newBounds);
            break;
    
        default:
            handled=Form::handleEvent(event);
    }
    return handled;
}