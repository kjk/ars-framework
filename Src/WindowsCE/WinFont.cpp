// WinFont.cpp: implementation of the WinFont class.
//
//////////////////////////////////////////////////////////////////////

#include "WinFont.h"
#include "FontEffects.hpp"

WinFont::WinFont()
{
    this->fntWrapper = new FontWrapper((HFONT)GetStockObject(SYSTEM_FONT));
}
WinFont WinFont::getSymbolFont()
{
    return WinFont(HFONT(GetStockObject(SYSTEM_FONT))); 
}

WinFont::WinFont(HFONT fnt)
{
    this->fntWrapper = new FontWrapper(fnt);
}

WinFont::WinFont(const WinFont& r)
{
    this->fntWrapper = r.fntWrapper;
    fntWrapper->attach();
}

WinFont& WinFont::operator=(const WinFont& r)
{
    if (this==&r) 
        return *this;
    fntWrapper->detach();
    if (!fntWrapper->getRefsCount())
        delete fntWrapper;
    this->fntWrapper = r.fntWrapper;
    this->fntWrapper->attach();
    return *this;
}

HFONT WinFont::getHandle() const
{
    return fntWrapper->font;
}

void WinFont::setEffects(ArsLexis::FontEffects& fx)
{
    
}

void WinFont::addEffects(ArsLexis::FontEffects& fx)
{
    
}

ArsLexis::FontEffects WinFont::effects() const
{
    return ArsLexis::FontEffects();
}


WinFont::~WinFont()
{
    fntWrapper->detach();
    if (!fntWrapper->getRefsCount())
        delete fntWrapper;
}

WinFont::FontWrapper::FontWrapper(HFONT fnt):
font(fnt),
refsCount(1)
{
    
}

WinFont::FontWrapper::~FontWrapper()
{
    DeleteObject(this->font);
}
