#include "GenericTextElement.hpp"
#include <BaseTypes.hpp>
#include <Text.hpp>
#include <memory>

using ArsLexis::char_t;
using ArsLexis::String;
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
    style_(styleDefault),
    actionCallback_(NULL),
    actionCallbackData_(NULL)
{}

GenericTextElement::GenericTextElement(const char_t* text):
    text_(text),
    hyperlink_(0),
    style_(styleDefault),
    actionCallback_(NULL),
    actionCallbackData_(NULL)
{}

GenericTextElement::~GenericTextElement()
{
    delete hyperlink_;
}

namespace {
    
    static uint_t findNextWhitespace(const String& text, uint_t fromPos)
    {
        uint_t length=text.length();
        for (uint_t i=fromPos; i<length; ++i)
            if (ArsLexis::isSpace(text[i]))
                return i;
        return length;
    }

    static uint_t whitespaceRangeLength(const String& text, uint_t start, uint_t length)
    {
        String::const_reverse_iterator it(text.rend()-start-length);
        return (text.rend()-std::find_if(it, it+length, ArsLexis::isSpace))-start;
    }

}

void GenericTextElement::drawTextWithSelection(Graphics& graphics, uint_t start, uint_t end, uint_t selectionStart, uint_t selectionEnd, const ArsLexis::Point& p)
{
    uint_t intersectStart=std::max(start, selectionStart);
    uint_t intersectEnd=std::min(end, selectionEnd);
    ArsLexis::Point point(p);
    uint_t length;
    const char_t* text;
    if (intersectStart<intersectEnd)
    {
        if (start<intersectStart)
        {
            graphics.drawText(text=(text_.c_str()+start), length=(intersectStart-start), point);
            point.x+=graphics.textWidth(text, length);
        }
        graphics.drawText(text=(text_.c_str()+intersectStart), length=(intersectEnd-intersectStart), point, true);
        point.x+=graphics.textWidth(text, length);
        if (intersectEnd<end)
        {
            graphics.drawText(text=(text_.c_str()+intersectEnd), length=(end-intersectEnd), point);
            point.x+=graphics.textWidth(text, length);
        }
    }
    else
        graphics.drawText(text=(text_.c_str()+start), length=(end-start), point);
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
    left += layoutContext.usedWidth;
    top += (layoutContext.baseLine-baseLine);
    
    if (layoutContext.isFirstInLine())
    {
        while (ArsLexis::isSpace(*text))
        {
            ++text;
            ++layoutContext.renderingProgress;
        }
    }

    uint_t nextWhitespace=findNextWhitespace(text_, layoutContext.renderingProgress)-layoutContext.renderingProgress;
    uint_t txtDx;

    uint_t length=graphics.wordWrap(text, layoutContext.availableWidth(), txtDx);
    if (0==layoutContext.renderingProgress && !layoutContext.isFirstInLine() && length<nextWhitespace)
    {
        uint_t newLineLength=graphics.wordWrap(text, layoutContext.screenWidth-indent);
        if (nextWhitespace<=newLineLength)
            return;
    }

    if (text_.length()==layoutContext.renderingProgress+length &&
        !ArsLexis::isSpace(text_[text_.length()-1]) &&
        layoutContext.nextTextElement && 
        !layoutContext.nextTextElement->breakBefore(layoutContext.preferences) &&
        !layoutContext.nextTextElement->text().empty() &&
        !ArsLexis::isSpace(layoutContext.nextTextElement->text()[0]))
    {
        LayoutContext copy(layoutContext.graphics, layoutContext.preferences, layoutContext.screenWidth);
        copy.baseLine=layoutContext.baseLine;
        copy.usedHeight=layoutContext.usedHeight;
        copy.usedWidth=layoutContext.usedWidth+txtDx;
        layoutContext.nextTextElement->calculateLayout(copy);
        if (0==copy.renderingProgress && !layoutContext.isFirstInLine()) 
        {
            uint_t rangeLength=whitespaceRangeLength(text_, layoutContext.renderingProgress, length);
            length=rangeLength;
            txtDx=graphics.textWidth(text, length);
        }
    }

    uint_t charsToDraw = length;
    uint_t curTxtDx = txtDx;
    while ( (charsToDraw>0) && 
        ArsLexis::isSpace(*(text+charsToDraw-1)) && 
        (layoutContext.availableWidth()<curTxtDx))
    {
        --charsToDraw;
        curTxtDx = graphics.textWidth(text, charsToDraw);
    }

    uint_t dispWidth = curTxtDx;

    if (dispWidth>layoutContext.availableWidth())
        return;

    if (render)
    {
        drawTextWithSelection(graphics, layoutContext.renderingProgress, layoutContext.renderingProgress+charsToDraw, 
            layoutContext.selectionStart, layoutContext.selectionEnd, Point(left, top));
        if (isHyperlink())
            defineHotSpot(*definition, ArsLexis::Rectangle(left, top, txtDx, lineHeight));
    }

    if (!render)
        layoutContext.extendHeight(lineHeight, baseLine);

    left += txtDx;    
    text += length;
    if (*text)
    {
        layoutContext.renderingProgress += length;
        layoutContext.usedWidth += txtDx;
/*        
        if (tryPacking)
        {
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
                    while (charsToDraw && ArsLexis::isSpace(*(text+charsToDraw-1)))
                        --charsToDraw;
                    drawTextWithSelection(graphics, layoutContext.renderingProgress, layoutContext.renderingProgress+charsToDraw, 
                        layoutContext.selectionStart, layoutContext.selectionEnd, Point(left, top));
//                    graphics.drawText(text, charsToDraw, Point(left, top));
                    if (isHyperlink())
                        defineHotSpot(*definition, ArsLexis::Rectangle(left, top, width, lineHeight));
                }
                
                layoutContext.renderingProgress+=length;
                layoutContext.usedWidth+=width;
            }
        }    */        
    }
    else
        layoutContext.markElementCompleted(txtDx);    
}

uint_t GenericTextElement::charIndexAtOffset(LayoutContext& lc, uint_t offset) {
    Graphics& graphics=lc.graphics;
    Graphics::StateSaver saveState(graphics);
    applyFormatting(graphics, lc.preferences);
    uint_t indent=indentation();
    lc.usedWidth=std::max(lc.usedWidth, indent);
    if (offset < lc.usedWidth)
        return lc.renderingProgress;
    const char_t* text=text_.c_str()+lc.renderingProgress;
    uint_t left = lc.usedWidth;
    if (lc.isFirstInLine())
    {
        while (ArsLexis::isSpace(*text))
        {
            ++text;
            ++lc.renderingProgress;
        }
    }
    uint_t nextWhitespace=findNextWhitespace(text_, lc.renderingProgress)-lc.renderingProgress;
    uint_t txtDx;
    uint_t length=graphics.wordWrap(text, lc.availableWidth(), txtDx);
    if (0 == lc.renderingProgress && !lc.isFirstInLine() && length < nextWhitespace)
    {
        uint_t newLineLength = graphics.wordWrap(text, lc.screenWidth - indent);
        if (nextWhitespace <= newLineLength)
            return lc.renderingProgress;
    }
    if (text_.length() == lc.renderingProgress + length &&
        !ArsLexis::isSpace(text_[text_.length()-1]) &&
        NULL != lc.nextTextElement && 
        !lc.nextTextElement->breakBefore(lc.preferences) &&
        !lc.nextTextElement->text().empty() &&
        !ArsLexis::isSpace(lc.nextTextElement->text()[0]))
    {
        LayoutContext copy(lc.graphics, lc.preferences, lc.screenWidth);
        copy.usedWidth = lc.usedWidth + txtDx;
        lc.nextTextElement->calculateLayout(copy);
        if (0 == copy.renderingProgress && !lc.isFirstInLine()) 
        {
            uint_t rangeLength = whitespaceRangeLength(text_, lc.renderingProgress, length);
            length = rangeLength;
            txtDx = graphics.textWidth(text, length);
        }
    }
    
    uint_t charsToDraw = length;
    uint_t curTxtDx = txtDx;
    while ( (charsToDraw>0) && 
        ArsLexis::isSpace(*(text  +charsToDraw - 1)) && 
        (lc.availableWidth() < curTxtDx))
    {
        --charsToDraw;
        curTxtDx = graphics.textWidth(text, charsToDraw);
    }

    uint_t dispWidth = curTxtDx;
    if (dispWidth > lc.availableWidth())
        return lc.renderingProgress;
    
    offset -= left;
    uint_t charIndex = charsToDraw;
    graphics.charsInWidth(text, charIndex, offset);
    
    text += length;
    if (*text) 
    {
        lc.renderingProgress += length;
        lc.usedWidth += txtDx;
    }
    else
        lc.markElementCompleted(txtDx);
    if (charIndex == charsToDraw)
        return offsetOutsideElement;
    return lc.renderingProgress + charsToDraw;
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
        ArsLexis::Font f=graphics.font();
        f.addEffects(decor.font.effects());
        graphics.setFont(f);
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

void GenericTextElement::defineHotSpot(Definition& definition, const ArsLexis::Rectangle& bounds)
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
    // actionCallback_ takes precedence over hyperlink handler and we don't
    // call hyperlink handler if actionCallback_ is present
    if (actionCallback_)
    {
        (*actionCallback_)(actionCallbackData_);
    }
    else
    {
        Definition::HyperlinkHandler* handler=definition.hyperlinkHandler();
        if (handler) 
            handler->handleHyperlink(definition, *this);
    }
}

void GenericTextElement::toText(String& appendTo, uint_t from, uint_t to) const
{
    if (LayoutContext::progressCompleted==to)
        to = String::npos;
    appendTo.append(text(), from, to);
}

