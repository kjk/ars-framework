#include <PopupMenu.hpp>

using namespace ArsLexis;

PopupMenu::PopupMenu(Form& form):
    renderer(form, prefs_),
    running_(false)
{
}

PopupMenu::~PopupMenu()
{
    if (running_)
        close();
}

void PopupMenu::close()
{
    if (!running_)
        return;
        
    Form& form = *renderer.form();
    form.removeObject(renderer.index());
    WinSetDrawWindow(form.windowHandle());
    WinSetActiveWindow(form.windowHandle());
    dispose();
    form.update();
    running_ = false;
}

Err PopupMenu::run(UInt16 id, const Rectangle& rect)
{
    assert(!running_);
    Form& form = *renderer.form();
    Err error = create(rect, simpleFrame, true, true);
    if (errNone != error)
        return error;
    WinSetActiveWindow(handle());
    WinSetDrawWindow(handle());
    WinDrawWindowFrame();
    
    Rectangle r(2, 2, rect.width() - 4, rect.height() - 4);
    UInt16 index = form.createGadget(id, r);
    if (frmInvalidObjectId == index)
    {
        error = memErrNotEnoughSpace;
        goto Cleanup;
    }
    renderer.attachByIndex(index);
    running_ = true;
    renderer.show();
    return errNone;
Cleanup:
    WinSetDrawWindow(form.windowHandle());
    WinSetActiveWindow(form.windowHandle());
    dispose();
    form.update();
    return error;
}

bool PopupMenu::handleEventInForm(EventType& event)
{
    if (!running_)
        return false;
        
    if (renderer.handleEventInForm(event))
        return true;
        
    if (penDownEvent == event.eType)
        close();
/*
    switch (event.eType)
    {
        case
    }
 */
 
    return false;
}
