// WinFont.h: interface for the WinFont class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WINFONT_H__4DB36623_0043_4054_B7FF_C3474967BB8E__INCLUDED_)
#define AFX_WINFONT_H__4DB36623_0043_4054_B7FF_C3474967BB8E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <wingdi.h>
#include "FontEffects.hpp"


class WinFont  
{
    class FontWrapper
    {
        friend class WinFont;
        FontWrapper(HFONT fnt);
        virtual ~FontWrapper();
        
    private:
        void attach() {refsCount++; }
        void detach() {refsCount--; }
        unsigned getRefsCount() {return refsCount;}
        FontWrapper();
        unsigned refsCount;
        HFONT font;
    };
    public:
        WinFont();
        WinFont(HFONT font);
        WinFont& operator=(const WinFont& r);
        void setEffects(ArsLexis::FontEffects& fx);
        WinFont(const WinFont& copy);
        void addEffects(ArsLexis::FontEffects& fx);
        ArsLexis::FontEffects effects() const;
        HFONT getHandle() const;
        virtual ~WinFont();
        static WinFont getSymbolFont();
    private:
        FontWrapper* fntWrapper;
};

#endif // !defined(AFX_WINFONT_H__4DB36623_0043_4054_B7FF_C3474967BB8E__INCLUDED_)
