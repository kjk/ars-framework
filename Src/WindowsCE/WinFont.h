#if !defined(_WINFONT_H__)
#define _WINFONT_H__

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

    // this one is for backwards compat with iNoah
    WinFont(HFONT handle)
    {
		// TODO: not necessarily true
		fontHeight_ = 11;
        handle_ = handle;
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

#endif
