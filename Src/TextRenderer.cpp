#include <TextRenderer.hpp>
#include <Application.hpp>
#include <68k/Hs.h>

static void ShowSelectionPopupMenu(const Point& point, const String& text, void* context)
{
    TextRenderer* renderer = static_cast<TextRenderer*>(context);
    if (NULL == renderer->selectionMenuModelBuilder)
        return;
    
    if (frmInvalidObjectId == renderer->selectionMenuId)
        return;
    
    PopupMenuModel* model = renderer->selectionMenuModelBuilder(text);
    PopupMenu menu(*renderer->form());
    menu.setModel(model);
    menu.hyperlinkHandler = renderer->hyperlinkHandler();
    menu.popup(renderer->selectionMenuId, point);
}

TextRenderer::RenderingErrorListener::~RenderingErrorListener() {}

TextRenderer::TextRenderer(Form& form, ScrollBar* scrollBar):
    FormGadget(form),
    scrollBar_(scrollBar),
    drawingWindow_(NULL),
    drawingWindowIsOffscreen_(false),
    lastRenderingError_(errNone),
    renderingErrorListener_(NULL),
    scheduledScrollDirection_(scheduledScrollAbandoned),
    selectionMenuModelBuilder(NULL),
    selectionMenuId(frmInvalidObjectId)
{
    setInteractionBehavior(  
        behavMouseSelection 
        | behavUpDownScroll 
        | behavHyperlinkNavigation 
//        | behavDoubleClickSelection
        | behavMenuBarCopyButton
    );
    definition_.setupSelectionClickHandler(ShowSelectionPopupMenu, this);
}

TextRenderer::~TextRenderer() {
    if (drawingWindowIsOffscreen_)
        disposeOffscreenWindow();
}

void TextRenderer::checkDrawingWindow()
{
    Form* form = this->form();
    Rect bounds;
    form->bounds(bounds);
    if (NULL != drawingWindow_ && drawingWindowBounds_ == bounds)
        return;
    if (drawingWindowIsOffscreen_)
        disposeOffscreenWindow();
    if (NULL == drawingWindow_)
    {
        assert(!drawingWindowIsOffscreen_);
        drawingWindow_ = form->windowHandle(); // WinGetDrawWindow(); // form->windowHandle();
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
//            bounds.explode(1, 1, -2, -2);
            formWindow.copyArea(bounds, offscreen, bounds.topLeft());
        }
    }
    assert(NULL != drawingWindow_);
}

void TextRenderer::updateForm(Graphics& graphics) 
{
    if (!drawingWindowIsOffscreen_)
        return;
    Rect bounds;
    this->bounds(bounds);
    //bounds.explode(1, 1, -2, -2);
    Graphics formWindow(form()->windowHandle()); //form()->windowHandle(); WinGetDrawWindow()
    graphics.copyArea(bounds, formWindow, bounds.topLeft());
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
        {
        ActivateGraphics activate(graphics);
        handleDraw(graphics);
        }
        if (errNone == lastRenderingError_)
            updateForm(graphics);
    }
    fireDrawCompleted();
    if (errNone == lastRenderingError_ && NULL != scrollBar_)
        doUpdateScrollbar();
   if (errNone != lastRenderingError_ && NULL != renderingErrorListener_)
        renderingErrorListener_->handleRenderingError(*this, lastRenderingError_);
}

void TextRenderer::calculateLayout()
{
    Graphics formWindow(form()->windowHandle()); 
    Rect bounds;
    this->bounds(bounds);
    definition_.calculateLayout(formWindow, bounds);    
}

void TextRenderer::drawRendererInBounds(Graphics& graphics, const Rect& bounds)
{
    lastRenderingError_ = definition_.render(graphics, bounds, false);
}

void TextRenderer::handleDraw(Graphics& graphics)
{
    Rect bounds;
    this->bounds(bounds);
    bounds.explode(1, 1, -2, -2);
    drawRendererInBounds(graphics, bounds);
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
        {
        ActivateGraphics activate(graphics);
        definition_.scroll(graphics, i);
        }
        if (errNone == lastRenderingError_)
            updateForm(graphics);
    }
    fireDrawCompleted();
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
    focus();
    return handleMouseEvent(event);
}

bool TextRenderer::handleMouseEvent(const EventType& event)
{
    UInt16 tapCount = 0;
    Point p(event.screenX, event.screenY);
    Rect bounds;
    this->bounds(bounds);
    bounds.explode(1, 1, -2, -2);
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
        else if (time >= scheduledNilEventTicks_)
        {
            EvtSetNullEventTick(time+1);
        } 
    }
    else
        scheduledScrollDirection_ = scheduledScrollAbandoned;
    checkDrawingWindow();
    {
        bool update;
        Graphics graphics(drawingWindow_);
        {
        ActivateGraphics activate(graphics);
        update = definition_.extendSelection(graphics, p, tapCount);
        }
        if (update)
            updateForm(graphics);
    }
    fireDrawCompleted();
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
        bool update;
        {
        ActivateGraphics activate(graphics);
        definition_.scroll(graphics, i);
        Rect bounds;
        this->bounds(bounds);
        bounds.explode(1, 1, -2, -2);
        Point p(bounds.topLeft());
        if (winDown == dir)
            p += bounds.extent_();
        update = definition_.extendSelection(graphics, p, 0);
        }
        if (update)
            updateForm(graphics);
    }
    fireDrawCompleted();
    if (NULL != scrollBar_)
        doUpdateScrollbar();
}

bool TextRenderer::handleNavigatorKey(Definition::NavigatorKey navKey) 
{
    bool handled = false;
    {
        checkDrawingWindow();
        Graphics graphics(drawingWindow_);
        {
        ActivateGraphics activate(graphics);
        handled = definition_.navigatorKey(graphics, navKey);
        }
        updateForm(graphics);
    }
    fireDrawCompleted();
    if (handled && NULL != scrollBar_)
        doUpdateScrollbar();
    return handled;
}

bool TextRenderer::handleKeyDownEvent(const EventType& event)
{
    if (form()->application().runningOnTreo600() && !hasFocus())
        return false;
    bool fieldHasFocus = false;
    UInt16 index = FrmGetFocus(*form());
    if (noFocus != index && frmFieldObj == FrmGetObjectType(*form(), index))
        fieldHasFocus = true;
    Definition::NavigatorKey key = Definition::NavigatorKey(-1);
    Form& form = *this->form();
    if (form.fiveWayUpPressed(&event) ||  chrUpArrow == event.data.keyDown.chr)
        key = Definition::navKeyUp;
    else if (form.fiveWayDownPressed(&event) ||  chrDownArrow == event.data.keyDown.chr)
        key = Definition::navKeyDown;
    else if (!fieldHasFocus && (form.fiveWayLeftPressed(&event) || chrLeftArrow == event.data.keyDown.chr))
        key = Definition::navKeyLeft;
    else if (!fieldHasFocus && (form.fiveWayRightPressed(&event) || chrRightArrow == event.data.keyDown.chr))
        key = Definition::navKeyRight;
    else if (form.fiveWayCenterPressed(&event) || chrLineFeed == event.data.keyDown.chr)
        key = Definition::navKeyCenter;
    if (Definition::NavigatorKey(-1) == key)
        return false;
    return handleNavigatorKey(key);
}

void TextRenderer::drawFocusRing()
{
    assert(hasFocus());
    assert(form()->application().runningOnTreo600());
    Rect rect;
    bounds(rect);
    if (NULL != scrollBar_) 
    {
        rect += scrollBar_->bounds();
    }
    Err error = HsNavDrawFocusRing(*form(), id(), hsNavFocusRingNoExtraInfo, &rect.native(), hsNavFocusRingStyleSquare, false);
    assert(errNone == error);
}

void TextRenderer::removeFocusRing()
{
    FormGadget::removeFocusRing();
}

bool TextRenderer::copySelectionOrAll() const
{
    String text;
    definition_.selectionOrAllToText(text);
    if (text.empty())
        return false;
    ClipboardAddItem(clipboardText, text.data(), text.length());
    return true;
}

bool TextRenderer::handleMenuCmdBarOpen(EventType& event)
{
    if (!usesMenuBarCopyButton())
        return false;
    if (!visible())
        return false;
    if (!definition_.hasSelection())
        return false;
    UInt16 focus = FrmGetFocus(*form());
    if (noFocus != focus)
    {
        FormObjectKind kind = FrmGetObjectType(*form(), focus);
        if (frmFieldObj == kind || frmTableObj == kind)
            return false;
    }
//    if (!hasFocus())
//        return false;
    event.data.menuCmdBarOpen.preventFieldButtons = true;
    Err error = MenuCmdBarAddButton(menuCmdBarOnLeft, BarCopyBitmap, menuCmdBarResultMenuItem, sysEditMenuCopyCmd, "Copy");
    return false;
}

bool TextRenderer::handleMenuEvent(const EventType& event)
{
    if (sysEditMenuCopyCmd != event.data.menu.itemID)
        return false;
    if (!usesMenuBarCopyButton())
        return false;
    if (!visible())
        return false;
    if (!definition_.hasSelection())
        return false;
    UInt16 focus = FrmGetFocus(*form());
    if (noFocus != focus)
    {
        FormObjectKind kind = FrmGetObjectType(*form(), focus);
        if (frmFieldObj == kind || frmTableObj == kind)
            return false;
    }
    return copySelectionOrAll();         
}

bool TextRenderer::handleEventInForm(EventType& event)
{
    bool handled = false;
    switch (event.eType)
    {
        case menuCmdBarOpenEvent:
            handled = handleMenuCmdBarOpen(event);
            break;
        
        case menuEvent:
            handled = handleMenuEvent(event);
            break;
            
        case keyDownEvent:
            handled = handleKeyDownEvent(event);
            break;
            
        case sclRepeatEvent:
            handleScrollRepeat(event);
            break;
    }
    return handled;            
}
