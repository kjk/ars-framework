#include <TextElement.hpp>
#include <Text.hpp>

TextElement::TextElement(const String& text):
    text_(text)
{}

TextElement::TextElement(const char_t* text):
    text_(text)
{}

TextElement::TextElement() {}

TextElement::~TextElement()
{
}

static uint_t findNextWhitespace(const String& text, uint_t fromPos)
{
    uint_t length=text.length();
    for (uint_t i=fromPos; i<length; ++i)
        if (isSpace(text[i]))
            return i;
    return length;
}

static uint_t whitespaceRangeLength(const String& text, uint_t start, uint_t length)
{
#if _MSC_VER == 1400
	
	// TODO: rewrite w/o reverse iterators
	return 0;
#else
    String::const_reverse_iterator it = text.rend() - start - length;
	String::const_reverse_iterator end = it + length;
    return (text.rend() - std::find_if(it, end, isSpace)) - start;
#endif
}

void TextElement::drawTextWithSelection(Graphics& graphics, uint_t start, uint_t end, uint_t selectionStart, uint_t selectionEnd, const ArsRectangle& area, bool hyperlink)
{
    uint_t intersectStart=std::max(start, selectionStart);
    uint_t intersectEnd=std::min(end, selectionEnd);
    Point point(area.topLeft);
    uint_t length;
    const char_t* text;
    if (intersectStart < intersectEnd)
    {
        if (start < intersectStart)
        {
            graphics.drawText(text = (text_.c_str() + start), length = (intersectStart-start), point);
            point.x += graphics.textWidth(text, length);
        }
#ifdef _PALM_OS
        if (!hyperlink) 
        {
            graphics.drawText(text=(text_.c_str()+intersectStart), length=(intersectEnd-intersectStart), point);
            ArsRectangle rect(point, Point(graphics.textWidth(text, length), area.height()));
            graphics.invertRectangle(rect);
            point.x += rect.width();
        }
        else {
#else
        if (true) 
        {
#endif
            graphics.drawText(text=(text_.c_str()+intersectStart), length=(intersectEnd-intersectStart), point, true);
            point.x+=graphics.textWidth(text, length);
        }
        if (intersectEnd<end)
        {
            graphics.drawText(text=(text_.c_str()+intersectEnd), length=(end-intersectEnd), point);
            point.x+=graphics.textWidth(text, length);
        }
    }
    else
        graphics.drawText(text=(text_.c_str()+start), length=(end-start), point);
}

void TextElement::calculateOrRender(LayoutContext& layoutContext, uint_t left, uint_t top, Definition* definition, bool render)
{
    assert(!layoutContext.isElementCompleted());
    Graphics& graphics=layoutContext.graphics;
    applyFormatting(graphics);

    uint_t indent=indentation();
    layoutContext.usedWidth=std::max(layoutContext.usedWidth, indent);
    uint_t baseLine=graphics.fontBaseline();
    uint_t lineHeight=graphics.fontHeight();
    
    const char_t* text=text_.c_str() + layoutContext.renderingProgress;
    left += layoutContext.usedWidth;
    top += (layoutContext.baseLine-baseLine);
    
    if (layoutContext.isFirstInLine())
    {
        while (isSpace(*text))
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
        !isSpace(text_[text_.length()-1]) &&
        layoutContext.nextTextElement && 
        !layoutContext.nextTextElement->breakBefore() &&
        !layoutContext.nextTextElement->text().empty() &&
        !isSpace(layoutContext.nextTextElement->text()[0]) &&
        justifyRightLastElementInLine != justification())
    {
        LayoutContext copy(layoutContext.graphics, layoutContext.screenWidth);
        copy.baseLine=layoutContext.baseLine;
        copy.usedHeight=layoutContext.usedHeight;
        copy.usedWidth=layoutContext.usedWidth+txtDx;
        layoutContext.nextTextElement->calculateLayout(copy);
        if (0==copy.renderingProgress && !layoutContext.isFirstInLine()) 
        {
            uint_t rangeLength=whitespaceRangeLength(text_, layoutContext.renderingProgress, length);
            length = rangeLength;
            txtDx = graphics.textWidth(text, length);
        }
    }

    uint_t charsToDraw = length;
    uint_t curTxtDx = txtDx;
    while ( (charsToDraw>0) && 
        isSpace(*(text+charsToDraw-1)) && 
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
        ArsRectangle textArea(left, top, txtDx, lineHeight);
        drawTextWithSelection(graphics, layoutContext.renderingProgress, layoutContext.renderingProgress+charsToDraw, 
            layoutContext.selectionStart, layoutContext.selectionEnd, textArea, layoutContext.selectionIsHyperlink);
        if (isHyperlink())
            defineHotSpot(*definition, textArea);
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
                    while (charsToDraw && isSpace(*(text+charsToDraw-1)))
                        --charsToDraw;
                    drawTextWithSelection(graphics, layoutContext.renderingProgress, layoutContext.renderingProgress+charsToDraw, 
                        layoutContext.selectionStart, layoutContext.selectionEnd, Point(left, top));
//                    graphics.drawText(text, charsToDraw, Point(left, top));
                    if (isHyperlink())
                        defineHotSpot(*definition, ArsRectangle(left, top, width, lineHeight));
                }
                
                layoutContext.renderingProgress+=length;
                layoutContext.usedWidth+=width;
            }
        }    */        
    }
    else
        layoutContext.markElementCompleted(txtDx);    
}

void TextElement::wordAtIndex(LayoutContext& lc, uint_t index, uint_t& wordStart, uint_t& wordEnd)
{
    wordStart = wordEnd = index;
    assert(offsetOutsideElement != index);
    uint_t length = text_.length();
    assert(index < length);
    if (!isAlNum(text_[index]))
        return;
    
    while (wordStart != 0 && isAlNum(text_[wordStart - 1]))
        --wordStart;

    while (wordEnd != length && isAlNum(text_[wordEnd]))
        ++wordEnd;
}


uint_t TextElement::charIndexAtOffset(LayoutContext& lc, uint_t offset)
{
    Graphics& graphics=lc.graphics;
    Graphics::StateSaver saveState(graphics);
    applyFormatting(graphics);
    uint_t indent=indentation();
    lc.usedWidth=std::max(lc.usedWidth, indent);
    if (offset < lc.usedWidth)
        return lc.renderingProgress;
    const char_t* text = text_.c_str()+lc.renderingProgress;
    uint_t left = lc.usedWidth;
    if (lc.isFirstInLine())
    {
        while (isSpace(*text))
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
        !isSpace(text_[text_.length()-1]) &&
        NULL != lc.nextTextElement && 
        !lc.nextTextElement->breakBefore() &&
        !lc.nextTextElement->text().empty() &&
        !isSpace(lc.nextTextElement->text()[0]) &&
        justifyRightLastElementInLine != justification())
    {
        LayoutContext copy(lc.graphics, lc.screenWidth);
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
        isSpace(*(text  +charsToDraw - 1)) && 
        (lc.availableWidth() < curTxtDx))
    {
        --charsToDraw;
        curTxtDx = graphics.textWidth(text, charsToDraw);
    }

    uint_t dispWidth = curTxtDx;
    if (dispWidth > lc.availableWidth())
        return lc.renderingProgress;
    
    offset -= left;
    ulong_t charIndex = charsToDraw;
    graphics.charsInWidth(text, charIndex, offset);
    
    text += length;
    uint_t lastProgress = lc.renderingProgress;
    if (*text) 
    {
        lc.renderingProgress += length;
        lc.usedWidth += txtDx;
    }
    else
        lc.markElementCompleted(txtDx);
    if (charIndex == charsToDraw)
        return offsetOutsideElement;
    return lastProgress + charIndex;
}

void TextElement::calculateLayout(LayoutContext& layoutContext)
{
    Graphics::StateSaver saveState(layoutContext.graphics);
    calculateOrRender(layoutContext, 0, 0);
}

void TextElement::render(RenderingContext& renderContext)
{
    Graphics::StateSaver saveState(renderContext.graphics);
    calculateOrRender(renderContext, renderContext.left, renderContext.top, &renderContext.definition, true);
}

void TextElement::applyFormatting(Graphics& graphics)
{
    graphics.applyStyle(getStyle(), isHyperlink());
}

void TextElement::toText(String& appendTo, uint_t from, uint_t to) const
{
    if (LayoutContext::progressCompleted == to)
        to = String::npos;
    appendTo.append(text(), from, to - from);
}

status_t TextElement::setText(const char_t* text, long len)
{
	text_.clear();
	return StringAppend(text_, text, len);
}
