#include "RegistrationForm.hpp"
#include "FormObject.hpp"
#include "iPediaApplication.hpp"

using ArsLexis::Rectangle;
using ArsLexis::FormObject;
using ArsLexis::Field;
using ArsLexis::Control;

void RegistrationForm::resize(const ArsLexis::Rectangle& screenBounds)
{
    
    Rectangle rect(2, 90, screenBounds.width()-4, 68);
    setBounds(rect);
    
    FormObject object(*this, serialNumberField);
    object.bounds(rect);
    rect.width()=screenBounds.width()-14;
    object.setBounds(rect);
    
    update();
}

Boolean RegistrationForm::handleOpen()
{
    Boolean handled=iPediaForm::handleOpen();
    
    Field field(*this, serialNumberField);

    iPediaApplication::Preferences& prefs=static_cast<iPediaApplication&>(application()).preferences();
    MemHandle handle=MemHandleNew(prefs.serialNumberLength+1);    if (handle)
    {
        char* text=static_cast<char*>(MemHandleLock(handle));
        if (text)
        {
            StrNCopy(text, prefs.serialNumber, prefs.serialNumberLength);            text[prefs.serialNumberLength]=chrNull;
            MemHandleUnlock(handle);
        }
        field.setText(handle);        
    }
    
    field.focus();
    return handled;
}

void RegistrationForm::handleControlSelect(const ctlSelect& data)
{
    if (okButton==data.controlID)
    {
        Field field(*this, serialNumberField);
        const char* text=field.text();
        if (NULL==text)
            text="";
        iPediaApplication::Preferences& prefs=static_cast<iPediaApplication&>(application()).preferences();
        if (0!=StrCompare(prefs.serialNumber, text))
        {
            uint_t len=StrLen(text);
            assert(len<=prefs.serialNumberLength);
            StrNCopy(prefs.serialNumber, text, prefs.serialNumberLength);
            prefs.serialNumber[prefs.serialNumberLength]=chrNull;
            prefs.serialNumberRegistered=false;
        }
    }
    closePopup();
}


Boolean RegistrationForm::handleEvent(EventType& event)
{
    Boolean handled=false;
    switch (event.eType)
    {
        case ctlSelectEvent:
            handleControlSelect(event.data.ctlSelect);
            handled=true;
            break;
            
        case keyDownEvent:
            if (chrCarriageReturn==event.data.keyDown.chr || chrLineFeed==event.data.keyDown.chr)
            {
                Control control(*this, okButton);
                control.hit();
                handled=true;
            }
            break;
    
        default:
            handled=iPediaForm::handleEvent(event);
    }
    return handled;
}