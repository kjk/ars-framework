#include <PalmOS.h>
#include <Graphics.hpp>

namespace ArsLexis
{

    Graphics::Font_t Graphics::setFont(const Graphics::Font_t& font)
    {
        Font_t oldOne=font_;
        font_=font;
        FntSetFont(font_.withEffects());
        effectiveLineHeight_=lineHeight_=FntLineHeight();
        FontEffects fx=font_.effects();
        if (fx.superscript() || fx.subscript())
        {
            effectiveLineHeight_*=4;
            effectiveLineHeight_/=3;
        }
        effectiveBaseline_=baseline_=FntBaseLine();
        if (font_.effects().superscript())
            effectiveBaseline_+=(lineHeight_/3);
        return oldOne;
    }

    namespace {
    
        class PalmUnderlineSetter
        {
            UnderlineModeType originalUnderline_;
        public:
            
            explicit PalmUnderlineSetter(UnderlineModeType newMode):
                originalUnderline_(WinSetUnderlineMode(newMode))
            {}
            
            ~PalmUnderlineSetter()
            {WinSetUnderlineMode(originalUnderline_);}
            
        };
        
        inline static UnderlineModeType convertUnderlineMode(FontEffects::Underline underline)
        {
            UnderlineModeType result=noUnderline;
            switch (underline)
            {
                case FontEffects::underlineDotted:
                    result=grayUnderline;
                    break;
                case FontEffects::underlineSolid:
                    result=solidUnderline;
                    break;
            }
            return result;                
        }
                
    }
        
    void Graphics::drawText(const char_t* text, uint_t length, const Point& topLeft, bool inverted)
    {
        FontEffects fx=font_.effects();    
        PalmUnderlineSetter setUnderline(convertUnderlineMode(fx.underline()));
        
        uint_t height=fontHeight();
        uint_t top=topLeft.y;
        if (fx.subscript())
            top+=(height/3);
        if (inverted)
            WinDrawInvertedChars(text, length, topLeft.x, top);
        else            
            WinDrawChars(text, length, topLeft.x, top);
        if (fx.strikeOut())
        {
            uint_t baseline=fontBaseline();
            top=topLeft.y+(baseline*2)/3;
            uint_t width=FntCharsWidth(text, length);
            Color_t color=setTextColor(0);
            setTextColor(color); // Quite strange method of querying current text color...
            color=setForegroundColor(color);
            WinDrawOperation old;
            if (inverted)
                old=WinSetDrawMode(winInvert);
            drawLine(topLeft.x, top, topLeft.x+width, top);
            if (inverted)
                WinSetDrawMode(old);
            setForegroundColor(color);
        }
    }

    void Graphics::charsInWidth(const char_t* text, uint_t& length, uint_t& width)
    {
        Int16 w=width;
        Int16 len=length;
        Boolean dontMind;
        FntCharsInWidth(text, &w, &len, &dontMind);
        length=len;
        width=w;
    }	        

    void Graphics::erase(const Rectangle& rect)
    {
        RectangleType nr=toNative(rect);
        WinEraseRectangle(&nr, 0);
    }
    
    void Graphics::copyArea(const Rectangle& sourceArea, Graphics& targetSystem, const Point& targetTopLeft)
    {
        RectangleType nr=toNative(sourceArea);
        WinCopyRectangle(handle_, targetSystem.handle_, &nr, targetTopLeft.x, targetTopLeft.y, winPaint);
    }

    Graphics::Graphics(const Handle_t& handle):
        handle_(handle),
        lineHeight_(0),
        effectiveLineHeight_(0),
        baseline_(0),
        effectiveBaseline_(0)
    {
        setFont(FntGetFont());
    }
 
    void Graphics::setClipping(const Rectangle& rect)
    {
        RectangleType nr=toNative(rect);
        WinSetClip(&nr);        
    }
    
    Graphics::ClipRectangleSetter::ClipRectangleSetter(Graphics& graphics, const Rectangle& rectangle):
        graphics_(graphics)
    {
        WinGetClip(&original_);
        graphics_.setClipping(rectangle);
    }
    
    Graphics::ClipRectangleSetter::~ClipRectangleSetter()
    {
        WinSetClip(&original_);        
    }
 
}