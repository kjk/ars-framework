#include "MainForm.hpp"
#include "DefinitionParser.hpp"
#include "iPediaApplication.hpp"
#include "iPediaConnection.hpp"
#include "SocketAddress.hpp"

using namespace ArsLexis;

MainForm::MainForm(iPediaApplication& app):
    iPediaForm(app, mainForm),
    displayMode_(showSplashScreen)
{
}

MainForm::~MainForm()
{
}


void MainForm::resize(const RectangleType& screenBounds)
{
    setBounds(screenBounds);

    RectangleType bounds;
    UInt16 index=getObjectIndex(definitionScrollBar);
    getObjectBounds(index, bounds);
    bounds.topLeft.x=screenBounds.extent.x-8;
    bounds.extent.y=screenBounds.extent.y-36;
    setObjectBounds(index, bounds);
    
    index=getObjectIndex(termLabel);
    getObjectBounds(index, bounds);
    bounds.topLeft.y=screenBounds.extent.y-14;
    setObjectBounds(index, bounds);

    index=getObjectIndex(termInputField);
    getObjectBounds(index, bounds);
    bounds.topLeft.y=screenBounds.extent.y-14;
    bounds.extent.x=screenBounds.extent.x-60;
    setObjectBounds(index, bounds);

    index=getObjectIndex(goButton);
    getObjectBounds(index, bounds);
    bounds.topLeft.x=screenBounds.extent.x-26;
    bounds.topLeft.y=screenBounds.extent.y-14;
    setObjectBounds(index, bounds);

    update();    
}

void MainForm::drawSplashScreen(Graphics& graphics, ArsLexis::Rectangle& bounds)
{
    UInt16 index=getObjectIndex(definitionScrollBar);
    hideObject(index);
    const iPediaApplication& app=static_cast<iPediaApplication&>(application());
    Graphics::ColorSetter setBackground(graphics, Graphics::colorBackground, app.renderingPreferences().backgroundColor());
    bounds.explode(0, 15, 0, -33);
    graphics.erase(bounds);
    UInt16 currentY=bounds.y()+20;
    Graphics::FontSetter font(graphics, largeFont);
    drawCenteredChars("ArsLexis iPedia", bounds.x(), currentY, bounds.width());
    currentY+=16;
#ifdef DEMO
    drawCenteredChars("Ver 0.5 (demo)", bounds.x(), currentY, bounds.width());
#else
  #ifdef DEBUG
    drawCenteredChars("Ver 0.5 (beta)", bounds.x(), currentY, bounds.width());
  #else
    drawCenteredChars("Ver 0.5", bounds.x(), currentY, bounds.width());
  #endif
#endif
    currentY+=20;
    
    font.changeTo(boldFont);
    drawCenteredChars("Copyright (c) ArsLexis", bounds.x(), currentY, bounds.width());
    currentY+=24;

    font.changeTo(largeFont);
    drawCenteredChars("http://www.arslexis.com", bounds.x(), currentY, bounds.width());
}

void MainForm::drawDefinition(Graphics& graphics, ArsLexis::Rectangle& bounds)
{
    const iPediaApplication& app=static_cast<iPediaApplication&>(application());
    Graphics::ColorSetter setBackground(graphics, Graphics::colorBackground, app.renderingPreferences().backgroundColor());
    bounds.explode(0, 15, 0, -33);
    graphics.erase(bounds);
    bounds.explode(2, 2, -12, -4);
    definition_.render(graphics, bounds, app.renderingPreferences());
    
    UInt16 index=getObjectIndex(definitionScrollBar);
    showObject(index);
    ScrollBarType* scrollBar=static_cast<ScrollBarType*>(getObject(index));
    SclSetScrollBar(scrollBar, definition_.firstShownLine(), 0, definition_.totalLinesCount()-definition_.shownLinesCount(), definition_.shownLinesCount());
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
        iPediaConnection* conn=new iPediaConnection(*manager);
        conn->setTransferTimeout(app.ticksPerSecond()*15L);
        conn->setTerm(newTerm);
        resolver->resolveAndConnect(conn, "localhost:9000");
    }
    else ;
        //! @todo Show alert that connection failed.
}

void MainForm::handleControlSelect(const ctlSelect& data)
{
    assert(data.controlID==goButton);
    UInt16 index=getObjectIndex(termInputField);
    const FieldType* field=static_cast<const FieldType*>(getObject(index));
    const char* textPtr=FldGetTextPtr(field);
    if (textPtr!=0)
    {
        String newTerm(textPtr);
        if (newTerm!=term())
            startLookupConnection(newTerm);
        else if (showDefinition!=displayMode())
        {
            setDisplayMode(showDefinition);
            update();
        }
    }
}


Boolean MainForm::handleEvent(EventType& event)
{
    Boolean handled=false;
    switch (event.eType)
    {
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