#include "MainForm.hpp"
#include "FormObject.hpp"
#include "DefinitionParser.hpp"
#include "iPediaApplication.hpp"
#include "iPediaConnection.hpp"
#include "SocketAddress.hpp"

using namespace ArsLexis;

#define serverLocalhost             "localhost:9000"
#define serverDictPcArslexis    "dict-pc.arslexis.com:9000"

MainForm::MainForm(iPediaApplication& app):
    iPediaForm(app, mainForm),
    displayMode_(showSplashScreen),
    server_(serverLocalhost)
{
}

MainForm::~MainForm()
{
}


void MainForm::resize(const ArsLexis::Rectangle& screenBounds)
{
    setBounds(screenBounds);

    Rectangle bounds;
    FormObject object(*this, definitionScrollBar);
    object.bounds(bounds);
    bounds.x()=screenBounds.extent.x-8;
    bounds.height()=screenBounds.extent.y-36;
    object.setBounds(bounds);
    
    object.attach(termInputField);
    object.bounds(bounds);
    bounds.y()=screenBounds.extent.y-14;
    bounds.width()=screenBounds.extent.x-55;
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

void MainForm::drawSplashScreen(Graphics& graphics, ArsLexis::Rectangle& bounds)
{
    FormObject object(*this, definitionScrollBar);
    object.hide();
    
    const iPediaApplication& app=static_cast<iPediaApplication&>(application());
    Graphics::ColorSetter setBackground(graphics, Graphics::colorBackground, app.renderingPreferences().backgroundColor());
    bounds.explode(0, 15, 0, -33);
    graphics.erase(bounds);
    
    Point point(bounds.x(), bounds.y()+20);
    
    PalmFont font(largeFont);
    
    Graphics::FontSetter setFont(graphics, font);
    graphics.drawCenteredText("ArsLexis iPedia", point, bounds.width());
    point.y+=16;
#ifdef DEMO
    graphics.drawCenteredText("Ver 0.5 (demo)", point, bounds.width());
#else
  #ifdef DEBUG
    graphics.drawCenteredText("Ver 0.5 (beta)", point, bounds.width());
  #else
    graphics.drawCenteredText("Ver 0.5", point, bounds.width());
  #endif
#endif
    point.y+=20;
    
    font.setFontId(stdFont);
    FontEffects fx;
    fx.setWeight(FontEffects::weightBold);
    font.setEffects(fx);

    setFont.changeTo(font);
    
    graphics.drawCenteredText("Copyright (c) ArsLexis", point, bounds.width());
    point.y+=24;
    
    fx.clear();
    font.setEffects(fx);
    
    setFont.changeTo(font);
    
    graphics.drawCenteredText("http://www.arslexis.com", point, bounds.width());
}

void MainForm::updateScrollBar()
{
    ScrollBar scrollBar(*this, definitionScrollBar);
    scrollBar.setPosition(definition_.firstShownLine(), 0, definition_.totalLinesCount()-definition_.shownLinesCount(), definition_.shownLinesCount());
    scrollBar.show();
}

void MainForm::drawDefinition(Graphics& graphics, ArsLexis::Rectangle& bounds)
{
    const iPediaApplication& app=static_cast<iPediaApplication&>(application());
    Graphics::ColorSetter setBackground(graphics, Graphics::colorBackground, app.renderingPreferences().backgroundColor());
    bounds.explode(0, 15, 0, -33);
    graphics.erase(bounds);
    bounds.explode(2, 2, -12, -4);
    definition_.render(graphics, bounds, app.renderingPreferences(), false);
    updateScrollBar();    
}

void MainForm::draw(UInt16 updateCode)
{
    iPediaForm::draw(updateCode);
    Rectangle rect=bounds();
    Graphics graphics(windowHandle());
    graphics.drawLine(rect.x(), rect.height()-18, rect.width(), rect.height()-18);
    if (showSplashScreen==displayMode())
        drawSplashScreen(graphics, rect);
    else
        drawDefinition(graphics, rect);
}


Err MainForm::initialize()
{
    Err error=iPediaForm::initialize();
    if (!error)
    {
        iPediaApplication& app=static_cast<iPediaApplication&>(application());
        definition_.setHyperlinkHandler(&app.hyperlinkHandler());
    }
    return error;
}

inline void MainForm::handleScrollRepeat(const EventType& event)
{
    iPediaApplication& app=static_cast<iPediaApplication&>(application());
    Graphics graphics(windowHandle());
    definition_.scroll(graphics, app.renderingPreferences(), event.data.sclRepeat.newValue-event.data.sclRepeat.value);
}

void MainForm::handlePenUp(const EventType& event)
{
    Point point(event.screenX, event.screenY);
    if (definition_.bounds() && point)
        definition_.hitTest(point); 
}

static void startLookupConnection(ArsLexis::Application& application, LookupHistory& history, const String& term, iPediaConnection::HistoryChange historyChange, const String& server)
{
    iPediaApplication& app=static_cast<iPediaApplication&>(application);
    SocketConnectionManager* manager=0;
    Err error=app.getConnectionManager(manager);
    if (!error)
    {
        Resolver* resolver=0;
        error=app.getResolver(resolver);
        assert(manager);
        assert(resolver);
        if (!manager->active())
        {
            iPediaConnection* conn=new iPediaConnection(*manager);
            conn->setTransferTimeout(app.ticksPerSecond()*15L);
            switch (historyChange)
            {
                case iPediaConnection::historyMoveBack:
                    conn->setTerm(history.previousTerm());
                    break;
                    
                case iPediaConnection::historyMoveForward:
                    conn->setTerm(history.nextTerm());
                    break;
                    
                case iPediaConnection::historyReplaceForward:
                    conn->setTerm(term);
                    break;
                    
                default:
                    assert(false);
            }
            conn->setHistoryChange(historyChange);
            resolver->resolveAndConnect(conn, server);
        }            
    }
}

void MainForm::startHistoryLookup(bool forward)
{
    ::startLookupConnection(application(), history(), String(), forward?iPediaConnection::historyMoveForward:iPediaConnection::historyMoveBack, server_);
}


void MainForm::startLookupConnection(const ArsLexis::String& newTerm)
{
    ::startLookupConnection(application(), history(), newTerm, iPediaConnection::historyReplaceForward, server_);
}

void MainForm::scrollDefinition(int units, MainForm::ScrollUnit unit)
{
    Graphics graphics(windowHandle());
    if (scrollPage==unit)
        units*=(definition_.shownLinesCount()/2);
    iPediaApplication& app=static_cast<iPediaApplication&>(application());
    definition_.scroll(graphics, app.renderingPreferences(), units);
    updateScrollBar();
}

void MainForm::lookupTerm(const ArsLexis::String& newTerm)
{
    if (history().empty() || newTerm!=history().currentTerm())
        startLookupConnection(newTerm);
    else if (showDefinition!=displayMode())
    {
        setDisplayMode(showDefinition);
        update();
    }
}

void MainForm::handleControlSelect(const EventType& event)
{
    switch (event.data.ctlSelect.controlID)
    {
        case searchButton:
            {
                Field field(*this, termInputField);
                const char* textPtr=field.text();
                if (textPtr!=0)
                    lookupTerm(textPtr);
            }
            break;
            
        case backButton:
            startHistoryLookup(false);
            break;
        
        case forwardButton:
            startHistoryLookup(true);
            break;
        
        default:
            assert(false);
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
            handled=true;
            break;
        
        case penUpEvent:
            handlePenUp(event);
            break;
            
        case sclRepeatEvent:
            handleScrollRepeat(event);
            break;
    
        default:
            handled=iPediaForm::handleEvent(event);
    }
    return handled;
}

void MainForm::synchronizeWithHistory()
{
    Field field(*this, currentTermField);
    if (!history().empty())
    {
        field.setText(history().currentTerm().c_str());
        field.show();
    }
    else
        field.hide();

    UInt16 buttonsWidth=0;
    Control control(*this, backButton);
    bool enabled=history().hasPrevious();
    control.setEnabled(enabled);
    control.setGraphics(enabled?backBitmap:backDisabledBitmap);
    
        
    control.attach(forwardButton);
    enabled=history().hasNext();
    control.setEnabled(enabled);
    control.setGraphics(enabled?forwardBitmap:forwardDisabledBitmap);
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

bool MainForm::handleMenuCommand(UInt16 itemId)
{
    bool handled=false;
    switch (itemId)
    {
        case useDictPcMenuItem:
            server_=serverDictPcArslexis;
            handled=true;
            break;
            
        case useLocalhostMenuItem:
            server_=serverLocalhost;
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
            
        default:
            handled=iPediaForm::handleMenuCommand(itemId);
    }
    return handled;
}

void MainForm::copySelectionToClipboard()
{
    if (showDefinition==displayMode())
    {
        ArsLexis::String text;
        definition_.selectionToText(text);
        ClipboardAddItem(clipboardText, text.data(), text.length());
    }
}

bool MainForm::handleWindowEnter(const struct _WinEnterEventType& data)
{
    const FormType* form=*this;
    if (data.enterWindow==static_cast<const void*>(form))
    {
        FormObject object(*this, termInputField);
        object.focus();
    }
    return iPediaForm::handleWindowEnter(data);
}
