#ifndef __ARSLEXIS_PALM_GRAPHICS_INLINE_HPP__
#define __ARSLEXIS_PALM_GRAPHICS_INLINE_HPP__

#ifndef INCLUDE_PALM_GRAPHICS_INLINE
#error "File PalmGraphicsInline.hpp should not be included directly. Use Graphics.hpp instead."
#endif

namespace ArsLexis
{

    inline void Graphics::erase(const Rectangle& rect)
    {
        NativeRectangle_t nr=toNative(rect);
        WinEraseRectangle(&nr, 0);
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
    {return support_.effectiveLineHeight;}
    
    inline uint_t Graphics::fontBaseline() const
    {return support_.effectiveBaseline;}

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

    inline Font Graphics::font() const
    {return support_.font;}
    
    inline void Graphics::copyArea(const Rectangle& sourceArea, Graphics& targetSystem, const Point& targetTopLeft)
    {
        NativeRectangle_t nr=toNative(sourceArea);
        WinCopyRectangle(handle_, targetSystem.handle_, &nr, targetTopLeft.x, targetTopLeft.y, winPaint);
    }

    inline Graphics::~Graphics()
    {}
    
    inline Graphics::Graphics(const NativeGraphicsHandle_t& handle):
        handle_(handle)
    {
        setFont(FntGetFont());
    }

    inline Graphics::State_t Graphics::pushState()
    {
        WinPushDrawState();
        return support_.font;
    }

    inline void Graphics::popState(const Graphics::State_t& state)
    {
        setFont(state);
        WinPopDrawState();
    }
    
    class ActivateGraphics: private NonCopyable
    {
        Graphics::Handle_t handle_;
    public:
        explicit ActivateGraphics(Graphics& g):
            handle_(WinSetDrawWindow(g.handle()))
        {}
        
        ~ActivateGraphics()
        {WinSetDrawWindow(handle_);}
        
    };
    
}

#endif