#include "MainForm.hpp"
#include "DefinitionParser.hpp"
#include "iPediaApplication.hpp"
#include "iPediaConnection.hpp"
#include "SocketAddress.hpp"

#include "FormattedTextElement.hpp"

using namespace ArsLexis;

MainForm::MainForm(iPediaApplication& app):
    iPediaForm(app, mainForm),
    displayMode_(showSplashScreen)
{
/*
    FormattedTextElement* ft;
    definition_.appendElement(ft=new FormattedTextElement("This is a piece of text written in standard font. "));
    definition_.appendElement(ft=new FormattedTextElement("This is striked out. "));
    ft->fontEffects().setStrikeOut(true);
    definition_.appendElement(ft=new FormattedTextElement("This is once again a piece of text written in standard font. "));
    definition_.appendElement(ft=new FormattedTextElement("This is underlined. "));
    ft->fontEffects().setUnderline(FontEffects::underlineSolid);    
    definition_.appendElement(ft=new FormattedTextElement("This is yet another piece of text written in standard font. "));
    definition_.appendElement(ft=new FormattedTextElement("Bold. "));
    ft->fontEffects().setWeight(FontEffects::weightBold);    
    definition_.appendElement(ft=new FormattedTextElement("This is next piece of text written in standard font. "));
    definition_.appendElement(ft=new FormattedTextElement("superscript "));
    ft->fontEffects().setSuperscript(true);
    definition_.appendElement(ft=new FormattedTextElement("Standard again. "));
    definition_.appendElement(ft=new FormattedTextElement("subscript "));
    ft->fontEffects().setSubscript(true);
    definition_.appendElement(ft=new FormattedTextElement("And finally standard once more. "));
    definition_.appendElement(ft=new FormattedTextElement("This is a piece of text written in standard font. "));
    ft->setStyle(styleHeader);
    definition_.appendElement(ft=new FormattedTextElement("This is striked out. "));
    ft->fontEffects().setStrikeOut(true);
    ft->setStyle(styleHeader);
    definition_.appendElement(ft=new FormattedTextElement("This is once again a piece of text written in standard font. "));
    ft->setStyle(styleHeader);
    definition_.appendElement(ft=new FormattedTextElement("This is underlined. "));
    ft->setStyle(styleHeader);
    ft->fontEffects().setUnderline(FontEffects::underlineSolid);    
    definition_.appendElement(ft=new FormattedTextElement("This is yet another piece of text written in standard font. "));
    ft->setStyle(styleHeader);
    definition_.appendElement(ft=new FormattedTextElement("Bold. "));
    ft->setStyle(styleHeader);
    ft->fontEffects().setWeight(FontEffects::weightBold);    
    definition_.appendElement(ft=new FormattedTextElement("This is next piece of text written in standard font. "));
    ft->setStyle(styleHeader);
    definition_.appendElement(ft=new FormattedTextElement("superscript "));
    ft->setStyle(styleHeader);
    ft->fontEffects().setSuperscript(true);
    definition_.appendElement(ft=new FormattedTextElement("Standard again. "));
    ft->setStyle(styleHeader);
    definition_.appendElement(ft=new FormattedTextElement("subscript "));
    ft->setStyle(styleHeader);
    ft->fontEffects().setSubscript(true);
    definition_.appendElement(ft=new FormattedTextElement("And finally standard once more. "));
    ft->setStyle(styleHeader);
    setDisplayMode(showDefinition);    
*/    
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
        if (!manager->active())
        {
            iPediaConnection* conn=new iPediaConnection(*manager);
            conn->setTransferTimeout(app.ticksPerSecond()*15L);
            conn->setTerm(newTerm);
            resolver->resolveAndConnect(conn, "localhost:9000");
        }            
    }
    else ;
        //! @todo Show alert that connection failed.
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
    UInt16 index=getObjectIndex(termInputField);
    const FieldType* field=static_cast<const FieldType*>(getObject(index));
    const char* textPtr=FldGetTextPtr(field);
    if (textPtr!=0)
        lookupTerm(textPtr);
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