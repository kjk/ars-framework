#ifndef __ARSLEXIS_PALM_GRAPHICS_INLINE_HPP__
#define __ARSLEXIS_PALM_GRAPHICS_INLINE_HPP__

namespace ArsLexis
{

    inline void Graphics::erase(const Rectangle& rect)
    {
        NativeRectangle_t nr=toNative<NativeRectangle_t, Rectangle>(rect);
        WinEraseRectangle(&nr, 0);
    }
    
    inline void Graphics::copyArea(const Rectangle& sourceArea, Graphics& targetSystem, const Point& targetTopLeft)
    {
        NativeRectangle_t nr=toNative<NativeRectangle_t, Rectangle>(sourceArea);
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
    
    inline NativeFont_t Graphics::setFont(NativeFont_t font)
    {
        return FntSetFont(font);
    }

}

#endif