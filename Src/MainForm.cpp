#include "MainForm.hpp"
#include "FormObject.hpp"
#include "iPediaApplication.hpp"
#include "LookupManager.hpp"

using namespace ArsLexis;

MainForm::MainForm(iPediaApplication& app):
    iPediaForm(app, mainForm),
    displayMode_(showSplashScreen)
{
}

MainForm::~MainForm()
{
}

Definition* MainForm::getDefinition()
{
    Definition* def=0;
    iPediaApplication& app=static_cast<iPediaApplication&>(application());
    LookupManager* lm=app.getLookupManager();
    if (lm)
        def=&lm->lastDefinition();
    return def;
}        

const LookupHistory* MainForm::getLookupHistory() const
{
    const LookupHistory* hist=0;
    const iPediaApplication& app=static_cast<const iPediaApplication&>(application());
    const LookupManager* lm=app.getLookupManager();
    if (lm)
        hist=&lm->history();
    return hist;
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

void MainForm::updateScrollBar(const Definition& definition)
{
    ScrollBar scrollBar(*this, definitionScrollBar);
    scrollBar.setPosition(definition.firstShownLine(), 0, definition.totalLinesCount()-definition.shownLinesCount(), definition.shownLinesCount());
    scrollBar.show();
}

void MainForm::drawDefinition(Graphics& graphics, const ArsLexis::Rectangle& bounds)
{
    Definition* definition=getDefinition();
    if (definition)
    {
        Graphics::ColorSetter setBackground(graphics, Graphics::colorBackground, renderingPreferences().backgroundColor());
        Rectangle rect(bounds);
        rect.explode(0, 15, 0, -33);
        graphics.erase(rect);
        rect.explode(2, 2, -12, -4);
        definition->render(graphics, rect, renderingPreferences(), false);
        updateScrollBar(*definition);    
    }
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
    Definition* definition=getDefinition();
    if (definition)
    {
        Graphics graphics(windowHandle());
        definition->scroll(graphics, renderingPreferences(), event.data.sclRepeat.newValue-event.data.sclRepeat.value);
    }        
}

void MainForm::handlePenUp(const EventType& event)
{
    Definition* definition=getDefinition();
    if (definition)
    {
        Point point(event.screenX, event.screenY);
        if (definition->bounds() && point)
            definition->hitTest(point);
    }
}

void MainForm::scrollDefinition(int units, MainForm::ScrollUnit unit)
{
    Definition* definition=getDefinition();
    if (definition)
    {
        Graphics graphics(windowHandle());
        if (scrollPage==unit)
            units*=(definition->shownLinesCount());
        definition->scroll(graphics, renderingPreferences(), units);
        updateScrollBar(*definition);
    }
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
                Field field(*this, termInputField);
                const char* textPtr=field.text();
                if (textPtr!=0)
                {
                    LookupManager* lookupManager=app.getLookupManager(true);
                    if (lookupManager && !lookupManager->lookupInProgress())
                        if (!lookupManager->lookupIfDifferent(textPtr) && showDefinition!=displayMode())
                            updateAfterLookup();
                }
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
            handlePenUp(event);
            break;
            
        case sclRepeatEvent:
            handleScrollRepeat(event);
            break;
            
        case iPediaApplication::appLookupFinishedEvent:
            {
                setControlsState(true);
                const LookupManager::LookupFinishedEventData& data=reinterpret_cast<const LookupManager::LookupFinishedEventData&>(event.data);
                if (data.outcomeDefinition==data.outcome)
                    updateAfterLookup();
                else if (data.outcomeList==data.outcome)
                    Application::popupForm(searchResultsForm);
            }
            break;     
            
        case iPediaApplication::appLookupStartedEvent:
            setControlsState(false);            // No break is intentional.
            
        case iPediaApplication::appLookupProgressEvent:
            update(redrawProgressIndicator);
            break;
    
        default:
            handled=iPediaForm::handleEvent(event);
    }
    return handled;
}

void MainForm::updateAfterLookup()
{
    const iPediaApplication& app=static_cast<const iPediaApplication&>(application());
    const LookupManager* lookupManager=app.getLookupManager();
    if (lookupManager)
    {
        setDisplayMode(showDefinition);
        const LookupHistory& history=lookupManager->history();
        if (!history.empty())
        {
            setTitle(history.currentTerm());
        }
        
        {
            Control control(*this, backButton);
            bool enabled=history.hasPrevious();
            control.setEnabled(enabled);
            control.setGraphics(enabled?backBitmap:backDisabledBitmap);
                
            control.attach(forwardButton);
            enabled=history.hasNext();
            control.setEnabled(enabled);
            control.setGraphics(enabled?forwardBitmap:forwardDisabledBitmap);
        }
        {
            Field field(*this, termInputField);        
            field.replaceText(lookupManager->lastInputTerm().c_str());
            field.selectWholeText();                    
        }    
        update();
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
        case useDictPcMenuItem:
            switchServer(serverDictPcArslexis);
            handled=true;
            break;
            
        case useLocalhostMenuItem:
            switchServer(serverLocalhost);
            handled=true;
            break;
            
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

        case aboutMenuItem:
            if (showSplashScreen!=displayMode())
            {
                setDisplayMode(showSplashScreen);
                update();
            }                
            handled=true;
            break;

        case toggleStressModeMenuItem:
            handleToggleStressMode();
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
    if (showDefinition==displayMode())
    {
        Definition* definition=getDefinition();
        if (definition)
        {
            ArsLexis::String text;
            definition->selectionToText(text);
            ClipboardAddItem(clipboardText, text.data(), text.length());
        }
    }
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
            if (!lookupManager->lastDefinition().empty())
                updateAfterLookup();
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
