#include <PalmOS.h>
#include <Graphics.hpp>

namespace ArsLexis
{

    Graphics::Font_t Graphics::setFont(const Graphics::Font_t& font)
    {
        Font_t oldOne=support_.font;
        support_.font=font;
        FntSetFont(support_.font.withEffects());
        if (oldOne.withEffects()!=font.withEffects() || 0==support_.effectiveLineHeight)
        {
            support_.effectiveLineHeight=support_.lineHeight=FntLineHeight();
            FontEffects fx=support_.font.effects();
            if (fx.superscript() || fx.subscript())
            {
                support_.effectiveLineHeight*=4;
                support_.effectiveLineHeight/=3;
            }
            support_.effectiveBaseline=support_.baseline=FntBaseLine();
            if (support_.font.effects().superscript())
                support_.effectiveBaseline+=(support_.lineHeight/3);
        }
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
        FontEffects fx=support_.font.effects();    
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
 
}