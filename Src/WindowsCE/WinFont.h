// WinFont.h: interface for the WinFont class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WINFONT_H__4DB36623_0043_4054_B7FF_C3474967BB8E__INCLUDED_)
#define AFX_WINFONT_H__4DB36623_0043_4054_B7FF_C3474967BB8E__INCLUDED_

#include "FontEffects.hpp"
#include <wingdi.h>

#define maxFontHeight 16;

class WinFont  
{
    private:
        
        static HFONT fontsHandles_[16-6][3][2][2];
        //Creates font with appropriate height and weight
        //(look at Fontffects::weight)
        static void createFont(int height, int weight, int realWeight, int isUnder, int isStrike);
        
        ArsLexis::FontEffects effects_;
        HFONT handle_;
        int   fontHeight_;
    public:
        
        static HFONT getFont(int height, ArsLexis::FontEffects effects);

        void setEffects(ArsLexis::FontEffects effects)
        {
            effects_ = effects;
            handle_ = getFont(fontHeight_, effects_);
        }
        
        void addEffects(ArsLexis::FontEffects& fx)
        {
            effects_ += fx;
            handle_ = getFont(fontHeight_, effects_);
        }
        
        ArsLexis::FontEffects effects() const
        {
            return effects_;
        }

        WinFont()
        {
            fontHeight_ = 11;
            handle_ = getFont(fontHeight_, effects_);
            
        }

        WinFont(int height)
        {
            fontHeight_ = height;
            handle_ = getFont(fontHeight_, effects_);
            
        }


        HFONT getHandle() const
        {
            return handle_;
        }
        
        static WinFont getSymbolFont()
        {
            return WinFont();
        }
};

#endif // !defined(AFX_WINFONT_H__4DB36623_0043_4054_B7FF_C3474967BB8E__INCLUDED_)
