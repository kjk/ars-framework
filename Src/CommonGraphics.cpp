#include <Graphics.hpp>

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
        // Graphics graphics;
        uint_t width = widthInOut;
        uint_t length = lengthInOut;
        //graphics.charsInWidth(textInOut.c_str(),length,width);
        charsInWidth(textInOut.c_str(),length,width);
        if (length == textInOut.length())
        {
            //fits in width
            widthInOut = width;
            lengthInOut = length;
            return;
        }
        //we need to add "..."
        uint_t ellipsisWidth = 100;
        uint_t ellipsisLength = 4;
        //graphics.charsInWidth(_T("..."),ellipsisLength,ellipsisWidth);
        charsInWidth(_T("..."),ellipsisLength,ellipsisWidth);
        width = widthInOut - ellipsisWidth;
        length = lengthInOut;
        if (!fFullWords)
        {
            //just wrap
            //graphics.charsInWidth(textInOut.c_str(),length,width);
            charsInWidth(textInOut.c_str(),length,width);
            textInOut.erase(length);
            textInOut.append(_T("..."));
            lengthInOut = textInOut.length();
            //graphics.charsInWidth(textInOut.c_str(),lengthInOut,widthInOut);
            charsInWidth(textInOut.c_str(),lengthInOut,widthInOut);
            return;
        }
        else
        {
            //wrap word
            //length = graphics.wordWrap2(textInOut.c_str(), width, width);
            length = wordWrap2(textInOut.c_str(), width, width);
            textInOut.erase(length);
            textInOut.append(_T("..."));
            lengthInOut = length + ellipsisLength;
            widthInOut = width + ellipsisWidth;
            return;    
        }
    }
   
}