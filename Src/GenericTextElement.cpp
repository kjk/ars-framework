#include "GenericTextElement.hpp"
#include <cctype>
#include <memory>

using ArsLexis::String;
using ArsLexis::char_t;
using ArsLexis::Rectangle;
using ArsLexis::Graphics;
using ArsLexis::Point;

GenericTextElement::HyperlinkProperties::HyperlinkProperties(const String& res, HyperlinkType t):
    resource(res),
    hotSpot(0),
    type(t)
{}

GenericTextElement::GenericTextElement(const String& text):
    text_(text),
    hyperlink_(0),
    style_(styleDefault)
{}

GenericTextElement::~GenericTextElement()
{
    delete hyperlink_;
}

#define whitespaceChars " \t\n\r"

#ifdef NDEBUG
inline 
#endif
static uint_t findNextWhitespace(const String& text, uint_t fromPos)
{
    String::size_type nextWhitespace=text.find_first_of(whitespaceChars, fromPos);
    if (String::npos==nextWhitespace)
        nextWhitespace=text.length();
    return nextWhitespace;
}

void GenericTextElement::calculateOrRender(LayoutContext& layoutContext, uint_t left, uint_t top, Definition* definition, bool render)
{
    assert(!layoutContext.isElementCompleted());
    Graphics& graphics=layoutContext.graphics;
    applyFormatting(graphics, layoutContext.preferences);

    uint_t indent=indentation();
    layoutContext.usedWidth=std::max(layoutContext.usedWidth, indent);
    uint_t baseLine=graphics.fontBaseline();
    uint_t lineHeight=graphics.fontHeight();
    
    const char_t* text=text_.c_str()+layoutContext.renderingProgress;
    left+=layoutContext.usedWidth;
    top+=(layoutContext.baseLine-baseLine);

    uint_t nextWhitespace=findNextWhitespace(text_, layoutContext.renderingProgress)-layoutContext.renderingProgress;
    uint_t length=graphics.wordWrap(text, layoutContext.availableWidth());
    if (0==layoutContext.renderingProgress && !layoutContext.isFirstInLine() && length<nextWhitespace)
    {
        uint_t newLineLength=graphics.wordWrap(text, layoutContext.screenWidth-indent);
        if (nextWhitespace<=newLineLength)
        {
            layoutContext.usedWidth+=layoutContext.availableWidth();
            return;
        }
    }

    if (!render)
        layoutContext.extendHeight(lineHeight, baseLine);

    uint_t width=graphics.textWidth(text, length);

    if (render)
    {
        uint_t charsToDraw=length;
        while (charsToDraw && std::isspace(*(text+charsToDraw-1)))
            --charsToDraw;
        graphics.drawText(text, charsToDraw, Point(left, top));
        if (isHyperlink())
            defineHotSpot(*definition, Rectangle(left, top, width, lineHeight));
    }

    left+=width;    
    text+=length;

    if (*text)
    {
        layoutContext.renderingProgress+=length;
        layoutContext.usedWidth+=width;
        nextWhitespace=findNextWhitespace(text_, layoutContext.renderingProgress)-layoutContext.renderingProgress;
        length=graphics.wordWrap(text, layoutContext.screenWidth-indent);
        if (length<nextWhitespace)
        {
            length=text_.length()-layoutContext.renderingProgress;
            width=layoutContext.availableWidth();
            graphics.charsInWidth(text, length, width);

            if (render)
            {
                uint_t charsToDraw=length;
                while (charsToDraw && std::isspace(*(text+charsToDraw-1)))
                    --charsToDraw;
                graphics.drawText(text, length, Point(left, top));
                if (isHyperlink())
                    defineHotSpot(*definition, Rectangle(left, top, width, lineHeight));
            }
            
            layoutContext.renderingProgress+=length;
            layoutContext.usedWidth+=layoutContext.availableWidth();
        }
    }
    else
        layoutContext.markElementCompleted(width);    
}

void GenericTextElement::calculateLayout(LayoutContext& layoutContext)
{
    Graphics::StateSaver saveState(layoutContext.graphics);
    calculateOrRender(layoutContext, 0, 0);
}

void GenericTextElement::render(RenderingContext& renderContext)
{
    Graphics::StateSaver saveState(renderContext.graphics);
    calculateOrRender(renderContext, renderContext.left, renderContext.top, &renderContext.definition, true);
}

void GenericTextElement::applyHyperlinkDecorations(Graphics& graphics, const RenderingPreferences& preferences)
{
    if (isHyperlink())
    {
        const RenderingPreferences::StyleFormatting& decor=preferences.hyperlinkDecoration(hyperlink_->type);
        graphics.font().addEffects(decor.font.effects());
        graphics.setTextColor(decor.textColor);
        graphics.setForegroundColor(decor.textColor);
    }
}

void GenericTextElement::applyFormatting(Graphics& graphics, const RenderingPreferences& preferences)
{
    const RenderingPreferences::StyleFormatting& format=preferences.styleFormatting(style_);
    graphics.setFont(format.font);
    graphics.setTextColor(format.textColor);
    applyHyperlinkDecorations(graphics, preferences);
}

void GenericTextElement::invalidateHotSpot()
{
    assert(isHyperlink());
    hyperlink_->hotSpot=0;
}

void GenericTextElement::defineHotSpot(Definition& definition, const Rectangle& bounds)
{
    assert(isHyperlink());
    if (!hyperlink_->hotSpot)
        definition.addHotSpot(hyperlink_->hotSpot=new Definition::HotSpot(bounds, *this));
    else
        hyperlink_->hotSpot->addRectangle(bounds);
}       

void GenericTextElement::setHyperlink(const String& resource, HyperlinkType type)
{
    if (!isHyperlink())
        hyperlink_=new HyperlinkProperties(resource, type);
    else
    {
        hyperlink_->resource=resource;
        hyperlink_->type=type;
    }
}

void GenericTextElement::performAction(Definition& definition)
{
    assert(isHyperlink());
    if (hyperlinkBookmark==hyperlink_->type)
        definition.goToBookmark(hyperlink_->resource);
    else
    {        
        Definition::HyperlinkHandler* handler=definition.hyperlinkHandler();
        if (handler)
            handler->handleHyperlink(hyperlink_->resource, hyperlink_->type);
    }
}
