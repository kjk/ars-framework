// WinFont.h: interface for the WinFont class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WINFONT_H__4DB36623_0043_4054_B7FF_C3474967BB8E__INCLUDED_)
#define AFX_WINFONT_H__4DB36623_0043_4054_B7FF_C3474967BB8E__INCLUDED_

#include "FontEffects.hpp"
#include <wingdi.h>


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
        ArsLexis::FontEffects effects_;
    };
    public:
        WinFont();
        WinFont(HFONT font);
        WinFont& operator=(const WinFont& r);
        WinFont(const WinFont& copy);

        void setEffects(ArsLexis::FontEffects effects)
        {fntWrapper->effects_=effects;}
        
        ArsLexis::FontEffects effects() const
        {return fntWrapper->effects_;}       
        
        void addEffects(ArsLexis::FontEffects& fx)
        {fntWrapper->effects_+=fx;}

        HFONT getHandle() const;
        virtual ~WinFont();
        static WinFont getSymbolFont();
    private:
        FontWrapper* fntWrapper;
};

#endif // !defined(AFX_WINFONT_H__4DB36623_0043_4054_B7FF_C3474967BB8E__INCLUDED_)
