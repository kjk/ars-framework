#ifndef __ARSLEXIS_PALM_GRAPHICS_INLINE_HPP__
#define __ARSLEXIS_PALM_GRAPHICS_INLINE_HPP__

namespace ArsLexis
{

    inline void Graphics::erase(const Rectangle& rect)
    {
        NativeRectangle_t nr=toNative(rect);
        WinEraseRectangle(&nr, 0);
    }
    
    inline void Graphics::copyArea(const Rectangle& sourceArea, Graphics& targetSystem, const Point& targetTopLeft)
    {
        NativeRectangle_t nr=toNative(sourceArea);
        WinCopyRectangle(handle_, targetSystem.handle_, &nr, targetTopLeft.x, targetTopLeft.y, winPaint);
    }


    inline void Graphics::drawLine(Coord_t x0, Coord_t y0, Coord_t x1, Coord_t y1)
    {
        WinDrawLine(x0, y0, x1, y1);
    }

    inline NativeColor_t Graphics::setForegroundColor(NativeColor_t color)
    {
        return WinSetForeColor(color);
    }
    
    inline NativeColor_t Graphics::setBackgroundColor(NativeColor_t color)
    {
        return WinSetBackColor(color);
    }
    
    inline NativeColor_t Graphics::setTextColor(NativeColor_t color)
    {
        return WinSetTextColor(color);
    }
    
    inline uint_t Graphics::fontHeight() const
    {
        uint_t height=FntLineHeight();
        if (support_.font.effects().superscript() || support_.font.effects().subscript())
            height*=1.333;
        return height;
    }
    
    inline uint_t Graphics::fontBaseline() const
    {
        uint_t baseline=FntBaseLine();
        if (support_.font.effects().superscript())
            baseline+=(FntLineHeight()*0.333);
        return baseline;
    }

    inline uint_t Graphics::wordWrap(const char_t* text, uint_t width)
    {
        return FntWordWrap(text, width);
    }

    inline uint_t Graphics::textWidth(const char_t* text, uint_t length)
    {
        return FntCharsWidth(text, length);
    }
    
    inline void Graphics::charsInWidth(const char_t* text, uint_t& length, uint_t& width)
    {
        Int16 w=width;
        Int16 len=length;
        Boolean dontMind;
        FntCharsInWidth(text, &w, &len, &dontMind);
        length=len;
        width=w;
    }

}

#endif