#ifndef _WINFONT_H_
#define _WINFONT_H_
#include <windows.h>
#include "FontEffects.hpp"

HFONT getFont(int height, FontEffects effects);

class WinFont  
{
    private:
    FontEffects effects_;
    HFONT       handle_;
    int         fontHeight_;

public:
    void setEffects(FontEffects effects)
    {
        effects_ = effects;
        handle_ = getFont(fontHeight_, effects_);
    }

    void addEffects(const FontEffects& fx)
    {
        effects_ += fx;
        handle_ = getFont(fontHeight_, effects_);
    }

    FontEffects effects() const
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
};

#endif
