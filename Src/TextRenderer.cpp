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
{}

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
        assert(drawingWindowIsOffscreen_);
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
        updateForm(graphics);
    }
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
        updateForm(graphics);
    }
    if (errNone != lastRenderingError_ && NULL != renderingErrorListener_)
        renderingErrorListener_->handleRenderingError(*this, lastRenderingError_);
}
