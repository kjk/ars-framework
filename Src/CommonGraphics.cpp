#include <Graphics.hpp>
#include <Text.hpp>

namespace ArsLexis
{
    
    namespace {
    
#ifdef __MWERKS__
#pragma pcrelconstdata on
#endif
    
        typedef Graphics::Color_t (Graphics::* ColorSetterMethod_t)(Graphics::Color_t);
        static ColorSetterMethod_t colorSetters[]={&Graphics::setTextColor, &Graphics::setForegroundColor, &Graphics::setBackgroundColor};

    }
    
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

    void Graphics::stripToWidthWithEllipsis(String& textInOut, uint_t& lengthInOut, uint_t& widthInOut, bool fFullWords)
    {
        uint_t width = widthInOut;
        uint_t length = lengthInOut;
        charsInWidth(textInOut.c_str(), length, width);
        if (length == textInOut.length())
        {
            //fits in width
            widthInOut = width;
            lengthInOut = length;
            return;
        }
        //we need to add "..."
        uint_t ellipsisWidth = 100;
        uint_t ellipsisLength = 3;
        charsInWidth(_T("..."), ellipsisLength, ellipsisWidth);
        width = widthInOut - ellipsisWidth;
        length = lengthInOut;
        if (!fFullWords)
        {
            //just wrap
            charsInWidth(textInOut.c_str(), length, width);
            textInOut.erase(length);
            textInOut.append(_T("..."));
            lengthInOut = textInOut.length();
            charsInWidth(textInOut.c_str(), lengthInOut, widthInOut);
            return;
        }
        else
        {
            //wrap word
            length = wordWrap(textInOut.c_str(), width, width);
            textInOut.erase(length);
            textInOut.append(_T("..."));
            lengthInOut = length + ellipsisLength;
            widthInOut = width + ellipsisWidth;
            return;    
        }
    }

    static const int drawTextInBoundsUseLinesValue = -1;

    void Graphics::drawTextInBounds(const String& text, const Rectangle& itemBounds, int totalLines, bool allowCenter)
    {
        drawTextInBoundsInternal(text, itemBounds,totalLines,allowCenter,drawTextInBoundsUseLinesValue);
    }
   
    void Graphics::drawTextInBoundsInternal(const String& text, const Rectangle& itemBounds, int totalLines, bool allowCenter, int lines)
    {
        if (drawTextInBoundsUseLinesValue == lines)
            lines = totalLines;
        assert (0 < lines);
        if (1 == lines)
        {
            uint_t width=itemBounds.width();
            uint_t length=text.length();
            charsInWidth(text.c_str(), length, width);
            drawText(text.c_str(), length, itemBounds.topLeft);
        }
        else
        {
            //draw what we can
            uint_t length = -1;
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
                uint_t len = tempLength;
                charsInWidth(text.c_str(), len, width);
                if (len < tempLength || width > (uint_t)itemBounds.width())
                    widthFull = true;
            }
            
            Rectangle newBounds = itemBounds;
            if (length == text.length() && totalLines == lines)
            {
                //all text in first line - center ?
                if (allowCenter)
                {
                    // TODO: FntLineHeight() is Palm-only, hence breaks wince build
                    // newBounds.topLeft.y += itemBounds.height()/2 - FntLineHeight()/2 -1;
                    newBounds.topLeft.y += itemBounds.height()/2 - fontHeight()/2 -1;
                }
                drawText(text.c_str(), length, newBounds.topLeft);
                return;
            }
            else
                drawText(text.c_str(), length, itemBounds.topLeft);
            
            //draw rest
            if (length < text.length())
            {
                String restOfText(text, length);
                strip(restOfText);
                Coord_t         heightChange = itemBounds.height()/lines;
                newBounds.topLeft.y += heightChange;
                newBounds.extent.y -= heightChange;
                drawTextInBoundsInternal(restOfText, newBounds, totalLines, false, lines-1);
            }    
        }
    }
}