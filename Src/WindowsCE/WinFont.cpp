// WinFont.cpp: implementation of the WinFont class.
//
//////////////////////////////////////////////////////////////////////

#include "WinFont.h"

WinFont::WinFont()
{
    LOGFONT logfnt;
    HFONT   fnt=(HFONT)GetStockObject(SYSTEM_FONT);
    GetObject(fnt, sizeof(logfnt), &logfnt);
    logfnt.lfHeight += 1;
	// do I need 2003 for this to work?
    // logfnt.lfQuality = CLEARTYPE_QUALITY;
    int fontDy = logfnt.lfHeight;
    HFONT fnt2=(HFONT)CreateFontIndirect(&logfnt);
    this->fntWrapper = new FontWrapper(fnt2);
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
    this->effects_=r.effects_;
    fntWrapper->attach();
}

WinFont& WinFont::operator=(const WinFont& r)
{
    if (this==&r) 
        return *this;
    this->effects_=r.effects_;
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
