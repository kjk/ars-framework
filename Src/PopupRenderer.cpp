#include <PopupRenderer.hpp>

using namespace ArsLexis;

PopupRenderer::PopupRenderer(Form& form, RenderingPreferences& prefs):
    TextRenderer(form, prefs),
    running_(false),
    prevFocusIndex_(frmInvalidObjectId)
{
}

PopupRenderer::~PopupRenderer()
{
    if (running_)
        close();
    
    if (valid())
    {
        form()->removeObject(index());
        form()->update();
    }
}

void PopupRenderer::close()
{
    if (!running_)
        return;
        
    hide();

    if (frmInvalidObjectId != prevFocusIndex_) 
    {
        FormObject object(*form());
        object.attachByIndex(prevFocusIndex_);
        object.focus();
    }
    
    form()->update();
    
    running_ = false;
}

Err PopupRenderer::run(UInt16 id, const Rectangle& rect)
{
    assert(!running_);
    Err error;
    prevFocusIndex_ = form()->focusedControlIndex_;
    form()->releaseFocus();
    

    UInt16 index = form()->createGadget(id, rect);
    if (frmInvalidObjectId == index)
    {
        error = memErrNotEnoughSpace;
        return error;
    }
    attachByIndex(index);
    running_ = true;
    show();
    focus();
     
    return errNone;
}

bool PopupRenderer::handleEventInForm(EventType& event)
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

void PopupRenderer::handleDraw(Graphics& graphics)
{
    Rectangle bounds;
    this->bounds(bounds);
    
    RectangleType r = toNative(bounds);
    WinEraseRectangle(&r, 0);
    r.topLeft.x++;
    r.topLeft.y++;
    r.extent.x-=2;
    r.extent.y-=2;
    WinDrawGrayRectangleFrame(simpleFrame, &r);

    bounds.explode(2, 2, -4, -4);
    drawRendererInBounds(graphics, bounds);
}

