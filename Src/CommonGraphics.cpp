#include <Graphics.hpp>
#include <Text.hpp>

#ifdef __MWERKS__
#pragma pcrelconstdata on
#endif

typedef Graphics::Color_t (Graphics::* ColorSetterMethod_t)(Graphics::Color_t);
static ColorSetterMethod_t colorSetters[]={&Graphics::setTextColor, &Graphics::setForegroundColor, &Graphics::setBackgroundColor};

Graphics::Color_t Graphics::setColor(Graphics::ColorChoice choice, Graphics::Color_t color)
{
    assert(choice<3);
    ColorSetterMethod_t setter=colorSetters[choice];
    return (this->*setter)(color);
}

void Graphics::drawCenteredText(const char_t* str, const Point& topLeft, uint_t width)
{
    assert(str!=0);
    uint_t len=tstrlen(str);
    Coord_t charsWidth=textWidth(str, len);
    Point point(topLeft);
    point.x += ((Coord_t)width-charsWidth)/2;
    drawText(str, len, point);
}

void Graphics::stripToWidthWithEllipsis(char_t* textInOut, ulong_t& lengthOut, uint_t& widthInOut, bool fFullWords)
{
    uint_t width = widthInOut;
    ulong_t origLen = tstrlen(textInOut);
    ulong_t newLen = origLen;
    charsInWidth(textInOut, newLen, width);
    if (newLen == origLen)
    {
        // everything fits
        widthInOut = width;
        lengthOut = origLen;
        return;
    }

    assert( newLen < origLen );


    // we need to add "..." (remember, that you can't add more than origLen!)
#if defined(_PALM_OS)
    const char ellipsis[] = {chrEllipsis, chrNull};
    uint_t ellipsisLength = 1;
#elif defined(_WIN32_WCE)
    const char_t ellipsis[] = {0x2026, 0};
    ulong_t ellipsisLength = 1;
#endif 
    
    uint_t ellipsisWidth = textWidth(ellipsis, ellipsisLength);
    width = widthInOut - ellipsisWidth;

    if (!fFullWords)
        charsInWidth(textInOut, newLen, width);
    else
        newLen = wordWrap(textInOut, width, width);

    if (origLen < newLen + ellipsisLength)
        newLen = origLen - 1;
        
    memmove(textInOut + newLen - 1, ellipsis, ellipsisLength + 1);
    charsInWidth(textInOut, lengthOut, widthInOut);
}

void Graphics::stripToWidthWithEllipsis(String& textInOut, ulong_t& lengthInOut, uint_t& widthInOut, bool fFullWords)
{
    uint_t width = widthInOut;
    ulong_t length = lengthInOut;
    charsInWidth(textInOut.c_str(), length, width);
    if (length == textInOut.length())
    {
        // fits in width
        widthInOut = width;
        lengthInOut = length;
        return;
    }
    
    // we need to add "..."
#if defined(_PALM_OS)
    const char ellipsis[] = {chrEllipsis, chrNull};
    ulong_t ellipsisLength = 1;
#elif defined(_WIN32_WCE)
    const char_t ellipsis[] = {0x2026, 0};
    ulong_t ellipsisLength = 1;
#endif 
    
    uint_t ellipsisWidth = textWidth(ellipsis, ellipsisLength);
    width = widthInOut - ellipsisWidth;
    length = lengthInOut;
    if (!fFullWords)
    {
        // just wrap
        charsInWidth(textInOut.c_str(), length, width);
        textInOut.erase(length);
        textInOut.append(ellipsis, ellipsisLength);
        lengthInOut = textInOut.length();
        charsInWidth(textInOut.c_str(), lengthInOut, widthInOut);
        return;
    }
    else
    {
        // wrap word
        length = wordWrap(textInOut.c_str(), width, width);
        textInOut.erase(length);
        textInOut.append(ellipsis, ellipsisLength);
        lengthInOut = length + ellipsisLength;
        widthInOut = width + ellipsisWidth;
        return;    
    }
}

static const int drawTextInBoundsUseLinesValue = -1;

void Graphics::drawTextInBounds(const String& text, const Rect& itemBounds, int totalLines, bool allowCenter)
{
    drawTextInBoundsInternal(text, itemBounds,totalLines,allowCenter,drawTextInBoundsUseLinesValue);
}

void Graphics::drawTextInBoundsInternal(const String& text, const Rect& itemBounds, int totalLines, bool allowCenter, int lines)
{
    if (drawTextInBoundsUseLinesValue == lines)
        lines = totalLines;
    assert (0 < lines);
    if (1 == lines)
    {
        uint_t width=itemBounds.width();
        ulong_t length=text.length();
        charsInWidth(text.c_str(), length, width);
        drawText(text.c_str(), length, itemBounds.topLeft());
    }
    else
    {
        //draw what we can
        ulong_t length = -1;
        uint_t tempLength = 0;
        bool widthFull = false;
        while (!widthFull && length!=tempLength)
        {
            length = tempLength;
            while (tempLength < text.length() && isSpace(text[tempLength]))
                tempLength++;
            while (tempLength < text.length() && !isSpace(text[tempLength]))
                tempLength++;
            
            uint_t width = itemBounds.width();
            ulong_t len = tempLength;
            charsInWidth(text.c_str(), len, width);
            if (len < tempLength || width > (uint_t)itemBounds.width())
                widthFull = true;
        }
        
        Rect newBounds = itemBounds;
        if (length == text.length() && totalLines == lines)
        {
            //all text in first line - center ?
            if (allowCenter)
                newBounds.topLeft().y += itemBounds.height()/2 - fontHeight()/2 -1;

            drawText(text.c_str(), length, newBounds.topLeft());
            return;
        }
        else
            drawText(text.c_str(), length, itemBounds.topLeft());
        
        //draw rest
        if (length < text.length())
        {
            String restOfText(text, 0, length);
            strip(restOfText);
            Coord_t         heightChange = itemBounds.height()/lines;
            newBounds.y() += heightChange;
            newBounds.setHeight(newBounds.height() - heightChange);
            drawTextInBoundsInternal(restOfText, newBounds, totalLines, false, lines-1);
        }    
    }
}
