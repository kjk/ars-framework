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
/*    
    WinSetDrawWindow(form.windowHandle());
    WinSetActiveWindow(form.windowHandle());
    dispose();
 */
    form.update();
    running_ = false;
}

void PopupMenu::draw()
{
    if (!running_)
        return;
    RectangleType rr = toNative(bounds_);
    WinEraseRectangle(&rr, 0);
    WinDrawGrayRectangleFrame(simpleFrame, &rr);

    Rectangle r = bounds_;
    r.explode(1, 1, -2, -2);
    renderer.draw();
}

Err PopupMenu::run(UInt16 id, const Rectangle& rect)
{
    assert(!running_);
    Form& form = *renderer.form();
/*    
    Err error = create(rect, simpleFrame, true, true);
    if (errNone != error)
        return error;
    WinSetActiveWindow(handle());
    WinSetDrawWindow(handle());
    WinDrawWindowFrame();
 */
    bounds_ = rect;
    Err error;
 
 
    Rectangle r = rect;
    r.explode(1, 1, -2, -2);
    UInt16 index = form.createGadget(id, r);
    if (frmInvalidObjectId == index)
    {
        error = memErrNotEnoughSpace;
        return error;
    }
    renderer.attachByIndex(index);
    renderer.show();
    running_ = true;
    draw();
    return errNone;
}

bool PopupMenu::handleEventInForm(EventType& event)
{
    if (!running_)
        return false;
        
    if (renderer.handleEventInForm(event))
        return true;
    
    if (penDownEvent == event.eType)
    {
        Point p(event.screenX, event.screenY);
        if (!bounds_.hitTest(p))
            close();
    }
/*
    switch (event.eType)
    {
        case
    }
 */
 
    return false;
}
