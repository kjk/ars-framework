#include "GenericTextElement.hpp"
#include "Utility.hpp"

using ArsLexis::String;
using ArsLexis::Rectangle;
using ArsLexis::isWhitespace;

GenericTextElement::HyperlinkProperties::HyperlinkProperties(const ArsLexis::String& res, HyperlinkType t):
    resource(res),
    hotSpot(0),
    type(t)
{}

GenericTextElement::GenericTextElement(const ArsLexis::String& text):
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
    applyFormatting(layoutContext.preferences);
    
    uint_t indent=indentation();
    if (layoutContext.usedWidth<indent)
        layoutContext.usedWidth=indent;

    uint_t baseLine=FntBaseLine();
    uint_t lineHeight=FntLineHeight();
    const char* text=text_.c_str()+layoutContext.renderingProgress;
    left+=layoutContext.usedWidth;
    top+=(layoutContext.baseLine-baseLine);

    uint_t nextWhitespace=findNextWhitespace(text_, layoutContext.renderingProgress)-layoutContext.renderingProgress;
    Int16 length=FntWordWrap(text, layoutContext.availableWidth());
    if (0==layoutContext.renderingProgress && !layoutContext.isFirstInLine() && length<nextWhitespace)
    {
        uint_t newLineLength=FntWordWrap(text, layoutContext.screenWidth);
        if (nextWhitespace<=newLineLength)
        {
            layoutContext.usedWidth+=layoutContext.availableWidth();
            return;
        }
    }

    if (!render)
        layoutContext.extendHeight(lineHeight, baseLine);

    Int16 width=FntCharsWidth(text, length);

    if (render)
    {
        uint_t charsToDraw=length;
        while (charsToDraw && isWhitespace(*(text+charsToDraw-1)))
            --charsToDraw;
        WinDrawChars(text, charsToDraw, left, top);
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
        length=FntWordWrap(text, layoutContext.screenWidth);
        if (length<nextWhitespace)
        {
            uint_t newLineLength=FntWordWrap(text, layoutContext.screenWidth);
            if (newLineLength<nextWhitespace)
            {
                length=StrLen(text);
                width=layoutContext.availableWidth();
                Boolean notTruncated=false;
                FntCharsInWidth(text, &width, &length, &notTruncated);

                if (render)
                {
                    uint_t charsToDraw=length;
                    while (charsToDraw && isWhitespace(*(text+charsToDraw-1)))
                        --charsToDraw;
                    WinDrawChars(text, length, left, top);
                    if (isHyperlink())
                        defineHotSpot(*definition, Rectangle(left, top, width, lineHeight));
                }
                
                layoutContext.renderingProgress+=length;
                layoutContext.usedWidth+=layoutContext.availableWidth();
            }
        }
    }
    else
        layoutContext.markElementCompleted(width);    
}

void GenericTextElement::calculateLayout(LayoutContext& layoutContext)
{
    WinPushDrawState();
    calculateOrRender(layoutContext, 0, 0);
    WinPopDrawState();
}

void GenericTextElement::render(RenderingContext& renderContext)
{
    WinPushDrawState();
    calculateOrRender(renderContext, renderContext.left, renderContext.top, &renderContext.definition, true);
    WinPopDrawState();
}

void GenericTextElement::applyHyperlinkDecorations(const RenderingPreferences& preferences)
{
    if (isHyperlink())
    {
        const RenderingPreferences::HyperlinkDecoration& decor=preferences.hyperlinkDecoration(hyperlink_->type);
        WinSetUnderlineMode(decor.underlineMode);
        WinSetTextColor(decor.textColor);
        WinSetForeColor(decor.textColor);
    }
}

void GenericTextElement::applyFormatting(const RenderingPreferences& preferences)
{
    const RenderingPreferences::StyleFormatting& format=preferences.styleFormatting(style_);
    FntSetFont(format.fontId);
    WinSetTextColor(format.textColor);
    WinSetForeColor(format.textColor);
    applyHyperlinkDecorations(preferences);
}

void GenericTextElement::invalidateHotSpot()
{
    assert(isHyperlink());
    hyperlink_->hotSpot=0;
}

void GenericTextElement::defineHotSpot(Definition& definition, const ArsLexis::Rectangle& bounds)
{
    assert(isHyperlink());
    if (!hyperlink_->hotSpot)
        definition.addHotSpot(hyperlink_->hotSpot=new Definition::HotSpot(bounds, *this));
    else
        hyperlink_->hotSpot->addRectangle(bounds);
}       

void GenericTextElement::setHyperlink(const ArsLexis::String& resource, HyperlinkType type)
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
