#include <TextRenderer.hpp>
#include <Application.hpp>

using namespace ArsLexis;

TextRenderer::RenderingErrorListener::~RenderingErrorListener() {}

TextRenderer::TextRenderer(Form& form, RenderingPreferences& prefs, ScrollBar* scrollBar):
    FormGadget(form),
    renderingPreferences_(prefs),
    scrollBar_(scrollBar),
    drawingWindow_(NULL),
    drawingWindowIsOffscreen_(false),
    lastRenderingError_(errNone),
    renderingErrorListener_(NULL)
{
    definition_.setInteractionBehavior(  
        Definition::behavMouseSelection 
        | Definition::behavUpDownScroll 
        | Definition::behavHyperlinkNavigation 
        | Definition::behavDoubleClickSelection
    );
}

TextRenderer::~TextRenderer() {
    if (NULL != drawingWindow_ && drawingWindowIsOffscreen_)
        WinDeleteWindow(drawingWindow_, false);
}

void TextRenderer::checkDrawingWindow()
{
    Form* form = this->form();
    Rectangle bounds;
    form->bounds(bounds);
    if (drawingWindowBounds_ == bounds)
        return;
    if (drawingWindowIsOffscreen_)
    {
        assert(NULL != drawingWindow_);
        WinDeleteWindow(drawingWindow_, false);
        drawingWindowIsOffscreen_ = false;
        drawingWindow_ = NULL;
    }
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
    if (penUpEvent == event.eType)
        tapCount = event.tapCount;
    checkDrawingWindow();
    Graphics graphics(drawingWindow_);
    ActivateGraphics activate(graphics);
    bool handled = definition_.extendSelection(graphics, renderingPreferences_, p, tapCount);
    updateForm(graphics);
    if (NULL != scrollBar_)
        doUpdateScrollbar();
    return handled;
}

