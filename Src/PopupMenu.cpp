#include <PopupMenu.hpp>

using namespace ArsLexis;

PopupMenu::PopupMenu(Form& form, RenderingPreferences& prefs):
    TextRenderer(form, prefs),
    running_(false),
    prevFocusIndex_(frmInvalidObjectId)
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
        
    if (frmInvalidObjectId != prevFocusIndex_) 
    {
        FormObject object(*form());
        object.attachByIndex(prevFocusIndex_);
        object.focus();
    }
    form()->removeObject(index());
    form()->update();
    
    running_ = false;
}

/*
void PopupMenu::draw()
{
    if (!running_)
        return;
    RectangleType rr = toNative(bounds_);
    WinEraseRectangle(&rr, 0);
    WinDrawGrayRectangleFrame(simpleFrame, &rr);
    
/*    
    PatternType p = WinGetPatternType();
    RGBColorType old;
    RGBColorType black;
    setRgbColor(black, 0, 0, 0);
    WinSetBackColorRGB(&black, &old);   
//      WinSetPatternType(grayPattern);
    rr.topLeft.x += rr.extent.x + 1;
    rr.topLeft.y += 4;
    rr.extent.x = 4;
    ++rr.extent.y;
    WinPaintRectangle(&rr, 0);
    rr.topLeft.x = bounds_.x() + 4;
    rr.topLeft.y = bounds_.y() + bounds_.height() + 1; 
    rr.extent.x = bounds_.width() - 3;
    rr.extent.y = 4;
    WinPaintRectangle(&rr, 0);
    WinSetPatternType(p);
    WinSetBackColorRGB(&old, NULL);
    
    renderer.draw();
}
*/

Err PopupMenu::run(UInt16 id, const Rectangle& rect)
{
    assert(!running_);
//    bounds_ = rect;
    Err error;
//    prevFocusIndex_ = form()->focusedControlIndex_;
//    form()->releaseFocus();
    
    Rectangle r = rect;
//    r.explode(1, 1, -2, -2);
    UInt16 index = form()->createGadget(id, r);
    if (frmInvalidObjectId == index)
    {
        error = memErrNotEnoughSpace;
        return error;
    }
    attachByIndex(index);
    running_ = true;
    show();
//    focus();
//    draw();
    return errNone;
}

bool PopupMenu::handleEventInForm(EventType& event)
{
    if (!running_)
        return false;
        
    if (TextRenderer::handleEventInForm(event))
        return true;
    
    switch (event.eType)
    {
        case penDownEvent:
        {
            Point p(event.screenX, event.screenY);
            if (!bounds().hitTest(p))
            {
                close();
                return true;
            }
            break;
        }
        
        case keyDownEvent:
        {
            if (form()->fiveWayUpPressed(&event) ||  chrUpArrow == event.data.keyDown.chr ||
                form()->fiveWayDownPressed(&event) ||  chrDownArrow == event.data.keyDown.chr ||
                form()->fiveWayLeftPressed(&event) || chrLeftArrow == event.data.keyDown.chr ||
                form()->fiveWayRightPressed(&event) || chrRightArrow == event.data.keyDown.chr)
                return true;
            return false;
        }
        
    }
    return false;
}

void PopupMenu::handleDraw(Graphics& graphics)
{
    Rectangle bounds;
    this->bounds(bounds);
    RectangleType r = toNative(bounds);
    WinEraseRectangle(&r, 0);
    WinDrawGrayRectangleFrame(simpleFrame, &r);

    bounds.explode(2, 2, -4, -4);
    drawRendererInBounds(graphics, bounds);
}

