#include "MainForm.hpp"
#include <FormObject.hpp>
#include "iPediaApplication.hpp"
#include "LookupManager.hpp"
#include "LookupHistory.hpp"
#include <SysUtils.hpp>

using namespace ArsLexis;

MainForm::MainForm(iPediaApplication& app):
    iPediaForm(app, mainForm),
    renderingProgressReporter_(*this),
    displayMode_(showSplashScreen),
    lastPenDownTimestamp_(0),
    updateDefinitionOnEntry_(false)
{
    definition_.setRenderingProgressReporter(&renderingProgressReporter_);
    definition_.setHyperlinkHandler(&app.hyperlinkHandler());
}

bool MainForm::handleOpen()
{
    bool result=iPediaForm::handleOpen();
    updateNavigationButtons();
    return result;
}

inline const LookupHistory& MainForm::getHistory() const
{
    return static_cast<const iPediaApplication&>(application()).history();
}

void MainForm::resize(const ArsLexis::Rectangle& screenBounds)
{
    Rectangle bounds(this->bounds());
    if (screenBounds!=bounds)
    {
        setBounds(screenBounds);

        FormObject object(*this, definitionScrollBar);
        object.bounds(bounds);
        bounds.x()=screenBounds.extent.x-8;
        bounds.height()=screenBounds.extent.y-36;
        object.setBounds(bounds);
        
        object.attach(termInputField);
        object.bounds(bounds);
        bounds.y()=screenBounds.extent.y-14;
        bounds.width()=screenBounds.extent.x-63;
        object.setBounds(bounds);

        object.attach(searchButton);
        object.bounds(bounds);
        bounds.x()=screenBounds.extent.x-34;
        bounds.y()=screenBounds.extent.y-14;
        object.setBounds(bounds);
        
        object.attach(backButton);
        object.bounds(bounds);
        bounds.y()=screenBounds.extent.y-14;
        object.setBounds(bounds);

        object.attach(forwardButton);
        object.bounds(bounds);
        bounds.y()=screenBounds.extent.y-14;
        object.setBounds(bounds);
            
        update();    
    }        
}

void MainForm::drawSplashScreen(Graphics& graphics, const ArsLexis::Rectangle& bounds)
{
    FormObject object(*this, definitionScrollBar);
    object.hide();
    
    const iPediaApplication& app=static_cast<iPediaApplication&>(application());
    Graphics::ColorSetter setBackground(graphics, Graphics::colorBackground, app.renderingPreferences().backgroundColor());
    Rectangle rect(bounds);
    rect.explode(0, 15, 0, -33);
    graphics.erase(rect);
    
    Point point(rect.x(), rect.y()+20);
    
    PalmFont font(largeFont);
    
    Graphics::FontSetter setFont(graphics, font);
    graphics.drawCenteredText("ArsLexis iPedia", point, rect.width());
    point.y+=16;
#ifdef DEMO
    graphics.drawCenteredText("Ver 0.5 (demo)", point, rect.width());
#else
  #ifdef DEBUG
    graphics.drawCenteredText("Ver 0.5 (beta)", point, rect.width());
  #else
    graphics.drawCenteredText("Ver 0.5", point, rect.width());
  #endif
#endif
    point.y+=20;
    
    font.setFontId(stdFont);
    FontEffects fx;
    fx.setWeight(FontEffects::weightBold);
    font.setEffects(fx);

    setFont.changeTo(font);
    
    graphics.drawCenteredText("Copyright (c) ArsLexis", point, rect.width());
    point.y+=24;
    
    fx.clear();
    font.setEffects(fx);
    
    setFont.changeTo(font);
    
    graphics.drawCenteredText("http://www.arslexis.com", point, rect.width());
}

void MainForm::updateScrollBar()
{
    ScrollBar scrollBar(*this, definitionScrollBar);
    scrollBar.setPosition(definition_.firstShownLine(), 0, definition_.totalLinesCount()-definition_.shownLinesCount(), definition_.shownLinesCount());
    scrollBar.show();
}

Err MainForm::renderDefinition(ArsLexis::Graphics& graphics, const ArsLexis::Rectangle& rect)
{
    volatile Err error=errNone;
    ErrTry {
        definition_.render(graphics, rect, renderingPreferences(), false);
    }
    ErrCatch (ex) {
        error=ex;
    } ErrEndCatch
    return error;
}


void MainForm::drawDefinition(Graphics& graphics, const ArsLexis::Rectangle& bounds)
{
    assert(!definition_.empty());
    Graphics::ColorSetter setBackground(graphics, Graphics::colorBackground, renderingPreferences().backgroundColor());
    Rectangle rect(bounds);
    rect.explode(0, 15, 0, -33);
    graphics.erase(rect);
    rect.explode(2, 2, -12, -4);
    Err error=errNone;
    bool doubleBuffer=true;
    const iPediaApplication& app=static_cast<const iPediaApplication&>(application());
    if (app.romVersionMajor()<5 && app.diaSupport() && app.diaSupport().hasSonySilkLib())
        doubleBuffer=false;
        
    if (doubleBuffer)
    {
        WinHandle wh=WinCreateOffscreenWindow(bounds.width(), bounds.height(), windowFormat(), &error);
        if (wh!=0)
        {
            {
            Graphics offscreen(wh);
            ActivateGraphics act(offscreen);
            error=renderDefinition(offscreen, rect);
            if (!error)
                offscreen.copyArea(rect, graphics, rect.topLeft);
            }
            WinDeleteWindow(wh, false);
        }
        else 
            doubleBuffer=false;
    }
    if (!doubleBuffer)
        error=renderDefinition(graphics, rect);
    if (errNone!=error) 
    {
        definition_.clear();
        setTitle(appName);
        setDisplayMode(showSplashScreen);
        update();
        iPediaApplication::sendDisplayAlertEvent(notEnoughMemoryAlert);
    }
    else            
        updateScrollBar();    
}

void MainForm::draw(UInt16 updateCode)
{
    Graphics graphics(windowHandle());
    Rectangle rect(bounds());
    Rectangle progressArea(rect.x(), rect.height()-17, rect.width(), 17);
    if (redrawAll==updateCode)
    {
        if (visible())
            graphics.erase(progressArea);
        iPediaForm::draw(updateCode);
        graphics.drawLine(rect.x(), rect.height()-18, rect.width(), rect.height()-18);
        if (showSplashScreen==displayMode())
            drawSplashScreen(graphics, rect);
        else
            drawDefinition(graphics, rect);
    }

    const iPediaApplication& app=static_cast<const iPediaApplication&>(application());
    const LookupManager* lookupManager=app.getLookupManager();
    if (lookupManager && lookupManager->lookupInProgress())
        lookupManager->showProgress(graphics, progressArea);
}


inline void MainForm::handleScrollRepeat(const EventType& event)
{
    scrollDefinition(event.data.sclRepeat.newValue-event.data.sclRepeat.value, scrollLine, false);
}

void MainForm::scrollDefinition(int units, MainForm::ScrollUnit unit, bool updateScrollbar)
{
    if (definition_.empty())
        return;
    WinHandle thisWindow=windowHandle();
    Graphics graphics(thisWindow);
    if (scrollPage==unit)
        units*=(definition_.shownLinesCount());
    
    bool doubleBuffer=true;
    if (-1==units || 1==units)
        doubleBuffer=false;
        
    const iPediaApplication& app=static_cast<const iPediaApplication&>(application());
    if (app.romVersionMajor()<5 && app.diaSupport() && app.diaSupport().hasSonySilkLib())
        doubleBuffer=false;

    if (doubleBuffer)
    {
        Rectangle b=bounds();
        Rectangle rect=b;
        rect.explode(2, 17, -12, -37);
        Err error=errNone;

        WinHandle wh=WinCreateOffscreenWindow(b.width(), b.height(), windowFormat(), &error);
        if (wh!=0)
        {
            {
            Graphics offscreen(wh);
            ActivateGraphics act(offscreen);
            graphics.copyArea(b, offscreen, Point(0, 0));
            definition_.scroll(offscreen, renderingPreferences(), units);
            offscreen.copyArea(b, graphics, Point(0, 0));
            }
            WinDeleteWindow(wh, false);
        }
        else
            doubleBuffer=false;
    }            
    if (!doubleBuffer)
        definition_.scroll(graphics, renderingPreferences(), units);
    if (updateScrollbar)
        updateScrollBar();
}

void MainForm::moveHistory(bool forward)
{
    iPediaApplication& app=static_cast<iPediaApplication&>(application());
    LookupManager* lookupManager=app.getLookupManager(true);
    if (lookupManager && !lookupManager->lookupInProgress())
        lookupManager->moveHistory(forward);
}


void MainForm::handleControlSelect(const EventType& event)
{
    iPediaApplication& app=static_cast<iPediaApplication&>(application());
    switch (event.data.ctlSelect.controlID)
    {
        case searchButton:
            {
                UInt32 timestamp=TimGetTicks();
                 // If button held for more than ~300msec, perform full text search.
                search(timestamp-lastPenDownTimestamp_>app.ticksPerSecond()/3);
            }                
            break;
            
        case backButton:
            moveHistory(false);
            break;
        
        case forwardButton:
            moveHistory(true);
            break;
        
        default:
            assert(false);
    }
}

void MainForm::setControlsState(bool enabled)
{
    {  // Scopes will allow compiler to optimize stack space allocating both form objects in the same place
        Control control(*this, backButton);
        control.setEnabled(enabled);
        control.attach(forwardButton);
        control.setEnabled(enabled);
        control.attach(searchButton);
        control.setEnabled(enabled);
    }
    {        
        Field field(*this, termInputField);
        if (enabled)
        {
            field.show();
            field.focus();
        }
        else
        {
            releaseFocus();
            field.hide();
        }
    }
}

void MainForm::handleLookupFinished(const EventType& event)
{
    setControlsState(true);
    const LookupManager::LookupFinishedEventData& data=reinterpret_cast<const LookupManager::LookupFinishedEventData&>(event.data);
    switch (data.outcome)
    {
        case data.outcomeDefinition:
            updateAfterLookup();
            break;
            
        case data.outcomeList:
            Application::popupForm(searchResultsForm);
            break;
            
        case data.outcomeNotFound:
            {
                Field field(*this, termInputField);
                field.select();
            }
            break;

        default:
            update();
    }
    
    iPediaApplication& app=iPediaApplication::instance();
    LookupManager* lookupManager=app.getLookupManager();
    assert(lookupManager);
    lookupManager->handleLookupFinishedInForm(data);

    if (app.inStressMode())
    {
        EventType event;
        MemSet(&event, sizeof(event), 0);
        event.eType=penDownEvent;
        event.penDown=true;
        event.tapCount=1;
        event.screenX=1;
        event.screenY=50;
        EvtAddEventToQueue(&event);
        MemSet(&event, sizeof(event), 0);
        event.eType=penUpEvent;
        event.penDown=false;
        event.tapCount=1;
        event.screenX=1;
        event.screenY=50;
        EvtAddEventToQueue(&event);
        randomArticle();
    }        
}

void MainForm::handleExtendSelection(const EventType& event, bool finish)
{
    if (showDefinition!=displayMode())
        return;
    iPediaApplication& app=static_cast<iPediaApplication&>(application());
    const LookupManager* lookupManager=app.getLookupManager();
    if (lookupManager && lookupManager->lookupInProgress())
        return;
    if (definition_.empty())
        return;
    ArsLexis::Point point(event.screenX, event.screenY);
    Graphics graphics(windowHandle());
    definition_.extendSelection(graphics, app.preferences().renderingPreferences, point, finish);
}

inline void MainForm::handlePenDown(const EventType& event)
{
    lastPenDownTimestamp_=TimGetTicks();
    handleExtendSelection(event);
}

bool MainForm::handleEvent(EventType& event)
{
    bool handled=false;
    switch (event.eType)
    {
        case keyDownEvent:
            handled=handleKeyPress(event);
            break;
            
        case ctlSelectEvent:
            handleControlSelect(event);
            break;
        
        case penUpEvent:
            handleExtendSelection(event, true);
            break;
    
        case penMoveEvent:
            handleExtendSelection(event);
            break;        
                
        case sclRepeatEvent:
            handleScrollRepeat(event);
            break;
            
        case iPediaApplication::appLookupFinishedEvent:
            handleLookupFinished(event);
            break;     
            
        case iPediaApplication::appLookupStartedEvent:
            setControlsState(false);            // No break is intentional.
            
        case iPediaApplication::appLookupProgressEvent:
            update(redrawProgressIndicator);
            break;
    
        case penDownEvent:
            handlePenDown(event);
            break;
    
        default:
            handled=iPediaForm::handleEvent(event);
    }
    return handled;
}

void MainForm::updateNavigationButtons()
{
    const LookupHistory& history=getHistory();

    Control control(*this, backButton);
    bool enabled=history.hasPrevious();
    control.setEnabled(enabled);
    control.setGraphics(enabled?backBitmap:backDisabledBitmap);
        
    control.attach(forwardButton);
    enabled=history.hasNext();
    control.setEnabled(enabled);
    control.setGraphics(enabled?forwardBitmap:forwardDisabledBitmap);
}


void MainForm::updateAfterLookup()
{
    iPediaApplication& app=static_cast<iPediaApplication&>(application());
    LookupManager* lookupManager=app.getLookupManager();
    assert(lookupManager!=0);
    if (lookupManager)
    {
        definition_.replaceElements(lookupManager->lastDefinitionElements());
        setDisplayMode(showDefinition);
        const LookupHistory& history=getHistory();
        if (history.hasCurrentTerm())
            setTitle(history.currentTerm());
        
        update();
        
        Field field(*this, termInputField);        
        field.replace(lookupManager->lastInputTerm());
        field.select();                    
    }
    updateNavigationButtons();
}

bool MainForm::handleKeyPress(const EventType& event)
{
    bool handled=false;

    switch (event.data.keyDown.chr)
    {
        case chrPageDown:
            if (displayMode()==showDefinition)
            {
                scrollDefinition(1, scrollPage);
                handled=true;
            }
            break;
            
        case chrPageUp:
            if (displayMode()==showDefinition)
            {
                scrollDefinition(-1, scrollPage);
                handled=true;
            }
            break;
        
        case chrDownArrow:
            if (displayMode()==showDefinition)
            {
                scrollDefinition(1, scrollLine);
                handled=true;
            }
            break;

        case chrUpArrow:
            if (displayMode()==showDefinition)
            {
                scrollDefinition(-1, scrollLine);
                handled=true;
            }
            break;
            
        case vchrRockerCenter:
        case chrLineFeed:
        case chrCarriageReturn:
            {
                lastPenDownTimestamp_=TimGetTicks();
                Control control(*this, searchButton);
                control.hit();
            }                
            handled=true;
            break;
    }
    return handled;
}

void MainForm::switchServer(const char* server)
{
    iPediaApplication& app=static_cast<iPediaApplication&>(application());
    app.setServer(server);    
}


bool MainForm::handleMenuCommand(UInt16 itemId)
{
    bool handled=false;
    switch (itemId)
    {
    
#ifdef  INTERNAL_BUILD    
        case useDictPcMenuItem:
            switchServer(serverDictPcArslexis);
            handled=true;
            break;
            
        case useLocalhostMenuItem:
            switchServer(serverLocalhost);
            handled=true;
            break;
#endif
            
        case registerMenuItem:
            Application::popupForm(registrationForm);
            handled=true;
            break;
            
        case copyMenuItem:
            copySelectionToClipboard();
            handled=true;
            break;
            
        case searchResultsMenuItem:
            Application::popupForm(searchResultsForm);
            handled=true;
            break;

        case randomMenuItem:
            randomArticle();
            handled=true;
            break;

		case arslexisWebsiteMenuItem:
            if ( errNone != ErrWebBrowserCommand(false, 0, sysAppLaunchCmdGoToURL, "http://www.arslexis.com/pda/ipedia.html",NULL) )
                FrmAlert(noWebBrowserAlert);
			handled = true;
			break;

        case aboutMenuItem:
            handleAbout();
            handled=true;
            break;

#ifdef INTERNAL_BUILD
        case toggleStressModeMenuItem:
            handleToggleStressMode();
            handled=true;
            break;
#endif            
            
        case searchMenuItem:
            search();
            handled=true;
            break;

        case fullTextSearchMenuItem:
            search(true);
            handled=true;
            break;
        
        
        default:
            handled=iPediaForm::handleMenuCommand(itemId);
    }
    return handled;
}

void MainForm::randomArticle()
{
    iPediaApplication& app=static_cast<iPediaApplication&>(application());
    LookupManager* lookupManager=app.getLookupManager(true);
    if (lookupManager && !lookupManager->lookupInProgress())
        lookupManager->lookupRandomTerm();
}

void MainForm::copySelectionToClipboard()
{
    if (showDefinition!=displayMode())
        return;
    if (definition_.empty())
        return;
    ArsLexis::String text;
    definition_.selectionToText(text);
    ClipboardAddItem(clipboardText, text.data(), text.length());
}

bool MainForm::handleWindowEnter(const struct _WinEnterEventType& data)
{
    const FormType* form=*this;
    if (data.enterWindow==static_cast<const void*>(form))
    {
        FormObject object(*this, termInputField);
        object.focus();
        
        iPediaApplication& app=static_cast<iPediaApplication&>(application());
        LookupManager* lookupManager=app.getLookupManager();
        if (lookupManager)
        {
            if (updateDefinitionOnEntry_)
            {
                updateDefinitionOnEntry_=false;
                updateAfterLookup();
            }
            setControlsState(!lookupManager->lookupInProgress());
        }
    }        
    return iPediaForm::handleWindowEnter(data);
}

void MainForm::handleToggleStressMode()
{
    iPediaApplication& app=static_cast<iPediaApplication&>(application());
    if (app.inStressMode())
        app.toggleStressMode(false);
    else
    {
        app.toggleStressMode(true);
        randomArticle();
    }        
}

void MainForm::handleAbout()
{
    if (showDefinition==displayMode())
    {
        setDisplayMode(showSplashScreen);
        update();
    }
    else 
    {
        if (!definition_.empty())
        {
            setDisplayMode(showDefinition);
            update();
        }
    }                
}

void MainForm::search(bool fullText)
{
    Field field(*this, termInputField);
    const char* text=field.text();
    uint_t textLen;
    if (0==text || 0==(textLen=StrLen(text)))
        return;
        
    iPediaApplication& app=static_cast<iPediaApplication&>(application());
    LookupManager* lookupManager=app.getLookupManager(true);
    if (!lookupManager || lookupManager->lookupInProgress())
        return;

    String term(text, textLen);
    if (!fullText)
    {
        if (!lookupManager->lookupIfDifferent(term) && showDefinition!=displayMode())
            updateAfterLookup();
    }
    else
        lookupManager->search(term);
}

MainForm::RenderingProgressReporter::RenderingProgressReporter(MainForm& form):
    form_(form),
    ticksAtStart_(0),
    lastPercent_(-1),
    showProgress_(false),
    afterTrigger_(false)
{

    Err error=ArsLexis::getResource(layoutProgressWaitText, waitText_);
    if (error || waitText_.empty())
        waitText_.assign("%d%%");
    waitText_.c_str(); // We don't want reallocation to occur while rendering...
}
        
#define IPEDIA_USES_TEXT_RENDERING_PROGRESS 0

void MainForm::RenderingProgressReporter::reportProgress(uint_t percent) 
{
    if (percent==lastPercent_)
        return;

    if (0==(lastPercent_=percent))
    {
        ticksAtStart_=TimGetTicks();
        showProgress_=false;
        afterTrigger_=false;
        return;
    }
    
    if (!afterTrigger_)
    {
        // Delay before we start displaying progress meter in milliseconds. Timespans < 300ms are typically perceived "instant"
        // So we shouldn't distract user if the time is short enough.
        static const uint_t delay=100; 
        UInt32 ticksDiff=TimGetTicks()-ticksAtStart_;
        iPediaApplication& app=static_cast<iPediaApplication&>(form_.application());
        ticksDiff*=1000;
        ticksDiff/=app.ticksPerSecond();
        if (ticksDiff>=delay)
            afterTrigger_=true;
        if (afterTrigger_ && percent<=20)
            showProgress_=true;
    }
    
    if (!showProgress_)
        return;
        
    Graphics graphics(form_.windowHandle());
    Rectangle bounds=form_.bounds();
    bounds.explode(2, 17, -12, -37);

    ActivateGraphics act(graphics);
#if IPEDIA_USES_TEXT_RENDERING_PROGRESS
    Font f;
    Graphics::FontSetter fset(graphics, f);
    uint_t height=graphics.fontHeight();
    Rectangle rect(bounds.x(), bounds.y()+(bounds.height()-height)/2, bounds.width(), height);
    graphics.erase(rect);
    char buffer[100];
    StrPrintF(buffer, waitText_.c_str(), percent);
    graphics.drawCenteredText(buffer, rect.topLeft, rect.width());
#else
    uint_t height=10;
    Rectangle rect(bounds.x()+16, bounds.y()+(bounds.height()-height)/2, bounds.width()-22, height);
    PatternType oldPattern=WinGetPatternType();
    WinSetPatternType(blackPattern);
    RectangleType nativeRec=toNative(rect);
    nativeRec.extent.x*=percent;
    nativeRec.extent.x/=100;
    WinPaintRectangle(&nativeRec, 0);
    nativeRec.topLeft.x+=nativeRec.extent.x;
    nativeRec.extent.x=rect.width()-nativeRec.extent.x;
    WinSetPatternType(grayPattern);
    WinPaintRectangle(&nativeRec, 0);
    WinSetPatternType(oldPattern);        
#endif    
}
