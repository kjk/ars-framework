#include <TextRenderer.hpp>
#include <Application.hpp>
#include <68k/Hs.h>

using namespace ArsLexis;

TextRenderer::RenderingErrorListener::~RenderingErrorListener() {}

TextRenderer::TextRenderer(Form& form, RenderingPreferences& prefs, ScrollBar* scrollBar):
    FormGadget(form),
    renderingPreferences_(prefs),
    scrollBar_(scrollBar),
    drawingWindow_(NULL),
    drawingWindowIsOffscreen_(false),
    lastRenderingError_(errNone),
    renderingErrorListener_(NULL),
    scheduledScrollDirection_(scheduledScrollAbandoned)
{
    definition_.setInteractionBehavior(  
        Definition::behavMouseSelection 
        | Definition::behavUpDownScroll 
        | Definition::behavHyperlinkNavigation 
        | Definition::behavDoubleClickSelection
    );
}

TextRenderer::~TextRenderer() {
    if (drawingWindowIsOffscreen_)
        disposeOffscreenWindow();
}

void TextRenderer::checkDrawingWindow()
{
    Form* form = this->form();
    Rectangle bounds;
    form->bounds(bounds);
    if (NULL != drawingWindow_ && drawingWindowBounds_ == bounds)
        return;
    if (drawingWindowIsOffscreen_)
        disposeOffscreenWindow();
    if (NULL == drawingWindow_)
    {
        assert(!drawingWindowIsOffscreen_);
        drawingWindow_ = form->windowHandle();
        drawingWindowBounds_ = bounds;
        Err error;
        WinHandle offscreenWindow = WinCreateOffscreenWindow(bounds.width(), bounds.height(), nativeFormat, &error);
        if (NULL != offscreenWindow)
        {
            Graphics formWindow(drawingWindow_);
            drawingWindow_ = offscreenWindow;
            drawingWindowIsOffscreen_ = true;
            Graphics offscreen(offscreenWindow);
            this->bounds(bounds);
            formWindow.copyArea(bounds, offscreen, bounds.topLeft);
        }
    }
    assert(NULL != drawingWindow_);
}

void TextRenderer::updateForm(Graphics& graphics) 
{
    if (!drawingWindowIsOffscreen_)
        return;
    Rectangle bounds;
    this->bounds(bounds);
    Graphics formWindow(form()->windowHandle());
    graphics.copyArea(bounds, formWindow, bounds.topLeft);
}

void TextRenderer::drawProxy()
{
    if (!visible())
        return;
    if (form()->application().runningOnTreo600() && hasFocus())
        drawFocusRing();
    checkDrawingWindow();
    {
        Graphics graphics(drawingWindow_);
        ActivateGraphics activate(graphics);
        handleDraw(graphics);
        if (errNone == lastRenderingError_)
            updateForm(graphics);
    }
    if (errNone == lastRenderingError_ && NULL != scrollBar_)
        doUpdateScrollbar();
   if (errNone != lastRenderingError_ && NULL != renderingErrorListener_)
        renderingErrorListener_->handleRenderingError(*this, lastRenderingError_);
}


void TextRenderer::handleDraw(Graphics& graphics)
{
    Rectangle bounds;
    this->bounds(bounds);
    lastRenderingError_ = definition_.render(graphics, bounds, renderingPreferences_, false);
}

void TextRenderer::scroll(WinDirectionType direction, uint_t items, ScrollbarUpdateOption update)
{
    if (0 == items)
        return;
    int i = items;
    if (winUp == direction)
        i = -i;
    checkDrawingWindow();
    {
        Graphics graphics(drawingWindow_);
        ActivateGraphics activate(graphics);
        definition_.scroll(graphics, renderingPreferences_, i);
        if (errNone == lastRenderingError_)
            updateForm(graphics);
    }
    if (errNone == lastRenderingError_ && updateScrollbar == update && NULL != scrollBar_)
        doUpdateScrollbar();
    if (errNone != lastRenderingError_ && NULL != renderingErrorListener_)
        renderingErrorListener_->handleRenderingError(*this, lastRenderingError_);
}

void TextRenderer::doUpdateScrollbar() 
{
    assert(NULL != scrollBar_);
    if (visibleLinesCount() == linesCount())
        scrollBar_->hide();
    else
    {
        uint_t visible = visibleLinesCount();
        uint_t total = linesCount();
        uint_t top = topLine();
        scrollBar_->setPosition(top, 0, total - visible, visible);
        scrollBar_->show();
        scrollBar_->draw();
    }    
}


bool TextRenderer::handleEvent(EventType& event) 
{
    bool handled = false;
    switch (event.eType)
    {
        case penMoveEvent:
        case penUpEvent:
            handled = handleMouseEvent(event);
            break;
        
        case nilEvent:
            handleNilEvent();
            break;
            
        default:
            handled = FormGadget::handleEvent(event);
    }
    return handled;
}

bool TextRenderer::handleEnter(const EventType& event)
{
    return handleMouseEvent(event);
}

bool TextRenderer::handleMouseEvent(const EventType& event)
{
    UInt16 tapCount = 0;
    Point p(event.screenX, event.screenY);
    Rectangle bounds;
    this->bounds(bounds);
    if (penUpEvent == event.eType)
        tapCount = event.tapCount;
    if (p && bounds)
        scheduledScrollDirection_ = scheduledScrollAbandoned;
    else if (p.x >= bounds.x() && p.x < bounds.x() + bounds.width())
    {
        UInt32 time = TimGetTicks();
        if (scheduledScrollAbandoned == scheduledScrollDirection_)
        {
            if (p.y < bounds.y())
                scheduledScrollDirection_ = scheduledScrollUp;
            else
                scheduledScrollDirection_ = scheduledScrollDown;
            scheduledNilEventTicks_ = time + form()->application().ticksPerSecond()/7;
            EvtSetNullEventTick(scheduledNilEventTicks_);
        }
    }
    else
        scheduledScrollDirection_ = scheduledScrollAbandoned;
    checkDrawingWindow();
    Graphics graphics(drawingWindow_);
    ActivateGraphics activate(graphics);
    definition_.extendSelection(graphics, renderingPreferences_, p, tapCount);
    updateForm(graphics);
    return true;
}

bool TextRenderer::handleScrollRepeat(const EventType& event)
{
    if (NULL == scrollBar_ || event.data.sclRepeat.scrollBarID != scrollBar_->id())
        return false;
    int items = event.data.sclRepeat.newValue - event.data.sclRepeat.value;
    WinDirectionType dir = winDown;
    if (0 > items)
    {
        items = -items;
        dir = winUp;
    }
    scroll(dir, items, updateScrollbarNot);
    return true;
}

void TextRenderer::disposeOffscreenWindow()
{
    assert(drawingWindowIsOffscreen_);
    assert(NULL != drawingWindow_);
    WinDeleteWindow(drawingWindow_, false);
    drawingWindowIsOffscreen_ = false;
    drawingWindow_ = NULL;
}

void TextRenderer::notifyHide() 
{
    if (drawingWindowIsOffscreen_)
        disposeOffscreenWindow();
}

void TextRenderer::handleNilEvent()
{
    if (scheduledScrollAbandoned == scheduledScrollDirection_)
        return;
    UInt32 time = TimGetTicks();
    if (time < scheduledNilEventTicks_)
    {
        EvtSetNullEventTick(scheduledNilEventTicks_);
        return;
    }
    WinDirectionType dir = winUp;
    int i = -1;
    if (scheduledScrollDown == scheduledScrollDirection_)
    {
        dir = winDown;
        i = 1;
    }
    scheduledNilEventTicks_ = time + form()->application().ticksPerSecond()/7;
    EvtSetNullEventTick(scheduledNilEventTicks_);
    checkDrawingWindow();
    Graphics graphics(drawingWindow_);
    {
        ActivateGraphics activate(graphics);
        definition_.scroll(graphics, renderingPreferences_, i);
        Rectangle bounds;
        this->bounds(bounds);
        Point p(bounds.topLeft);
        if (winDown == dir)
            p += bounds.extent;
        definition_.extendSelection(graphics, renderingPreferences_, p, 0);
        updateForm(graphics);
    }
    if (NULL != scrollBar_)
        doUpdateScrollbar();
}

bool TextRenderer::handleKeyDownEvent(const EventType& event)
{
    Definition::NavigatorKey key = Definition::NavigatorKey(-1);
    Form& form = *this->form();
    if (form.fiveWayUpPressed(&event))
        key = Definition::navKeyUp;
    else if (form.fiveWayDownPressed(&event))
        key = Definition::navKeyDown;
    else if (form.fiveWayLeftPressed(&event))
        key = Definition::navKeyLeft;
    else if (form.fiveWayRightPressed(&event))
        key = Definition::navKeyRight;
    else if (form.fiveWayCenterPressed(&event))
        key = Definition::navKeyCenter;
    if (Definition::NavigatorKey(-1) != key)
        return definition_.navigatorKey(key);
    return false;
}

void TextRenderer::drawFocusRing()
{
    assert(hasFocus());
    assert(form()->application().runningOnTreo600());
    RectangleType rect;
    FrmGetObjectBounds(*form(), index(), &rect);
    Err error = HsNavDrawFocusRing(*form(), id(), hsNavFocusRingNoExtraInfo, &rect, hsNavFocusRingStyleSquare, false);
    assert(errNone == error);
}

void TextRenderer::removeFocusRing()
{
    assert(form()->application().runningOnTreo600());
    HsNavRemoveFocusRing(*form());
}

