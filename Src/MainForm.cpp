#include "MainForm.hpp"
#include "FormObject.hpp"
#include "DefinitionParser.hpp"
#include "iPediaApplication.hpp"
#include "iPediaConnection.hpp"
#include "SocketAddress.hpp"

using namespace ArsLexis;

MainForm::MainForm(iPediaApplication& app):
    iPediaForm(app, mainForm),
    displayMode_(showSplashScreen)//,
//    historyPosition_(termHistory_.begin())
{
}

MainForm::~MainForm()
{
}

Boolean MainForm::handleOpen()
{
    FormObject object(*this, termInputField);
    object.focus();
    return iPediaForm::handleOpen();
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
    
    object.attach(termLabel);
    object.bounds(bounds);
    bounds.y()=screenBounds.extent.y-14;
    object.setBounds(bounds);

    object.attach(termInputField);
    object.bounds(bounds);
    bounds.y()=screenBounds.extent.y-14;
    bounds.width()=screenBounds.extent.x-73;
    object.setBounds(bounds);

    object.attach(goButton);
    object.bounds(bounds);
    bounds.x()=screenBounds.extent.x-26;
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
    definition_.render(graphics, bounds, app.renderingPreferences());
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

inline void MainForm::handleScrollRepeat(const sclRepeat& data)
{
    Graphics graphics(windowHandle());
    definition_.scroll(graphics, data.newValue-data.value);
}

void MainForm::handlePenUp(const EventType& event)
{
    Point point(event.screenX, event.screenY);
    if (definition_.bounds() && point)
        definition_.hitTest(point); 
}

void MainForm::startLookupConnection(const ArsLexis::String& newTerm)
{
    iPediaApplication& app=static_cast<iPediaApplication&>(application());
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
            conn->setTerm(newTerm);
            resolver->resolveAndConnect(conn, "localhost:9000");
        }            
    }
}

void MainForm::scrollDefinition(int units, MainForm::ScrollUnit unit)
{
    Graphics graphics(windowHandle());
    if (scrollPage==unit)
        units*=(definition_.shownLinesCount()/2);    definition_.scroll(graphics, units);
    updateScrollBar();
}


void MainForm::lookupTerm(const ArsLexis::String& newTerm)
{
    if (newTerm!=term())
        startLookupConnection(newTerm);
    else if (showDefinition!=displayMode())
    {
        setDisplayMode(showDefinition);
        update();
    }
}

void MainForm::handleControlSelect(const ctlSelect& data)
{
    assert(data.controlID==goButton);
    Field field(*this, termInputField);
    const char* textPtr=field.text();
    if (textPtr!=0)
        lookupTerm(textPtr);
}

Boolean MainForm::handleEvent(EventType& event)
{
    Boolean handled=false;
    switch (event.eType)
    {
        case keyDownEvent:
            handled=handleKeyPress(event);
            break;
            
        case ctlSelectEvent:
            handleControlSelect(event.data.ctlSelect);
            break;
        
        case penUpEvent:
            handlePenUp(event);
            break;
            
        case sclRepeatEvent:
            handleScrollRepeat(event.data.sclRepeat);
            break;
    
        default:
            handled=iPediaForm::handleEvent(event);
    }
    return handled;
}

void MainForm::updateCurrentTermField()
{
    Field field(*this, currentTermField);
    field.setText(term_.c_str());
    field.draw();
}

void MainForm::setTerm(const ArsLexis::String& term)
{
    term_=term;
    updateCurrentTermField();
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
            Control control(*this, goButton);
            control.hit();
            handled=true;
            break;
    }
    return handled;
}
