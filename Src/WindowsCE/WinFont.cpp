#include "WinFont.h"

using namespace ArsLexis;

//400 bytes to speed up operation - 
//in fact it can be hash table, because it's quite rare
HFONT WinFont::fontsHandles_[16-6][3][2][2];

void WinFont::createFont(int height, int weight, int realWeight, int isUnder, int isStrike)
{
    LOGFONT logfnt;
    HFONT   fnt=(HFONT)GetStockObject(SYSTEM_FONT);    
    GetObject(fnt, sizeof(logfnt), &logfnt);
    logfnt.lfWeight = realWeight;
    logfnt.lfHeight = -height;
    if(isUnder)
        logfnt.lfUnderline = TRUE;
    if(isStrike)
        logfnt.lfStrikeOut = TRUE;
    assert(height<maxFontHeight);
    fontsHandles_[height-6][weight][isUnder][isStrike] = CreateFontIndirect(&logfnt);
    if(NULL == fontsHandles_[height-6][weight][isUnder][isStrike])
        fontsHandles_[height-6][weight][isUnder][isStrike] = fnt;
}
        
HFONT WinFont::getFont(int height, ArsLexis::FontEffects effects)
{
    int weight = 0;
    int underline = 0;
    int strike = 0;
    LONG realWeight = FW_NORMAL;

    if(effects.underline()!=FontEffects::underlineNone)
        underline = 1;
    if(effects.strikeOut())
        strike = 1;
    switch(effects.weight())
    {
        case FontEffects::weightPlain:
            if (effects.italic())
            {
                weight = 1;
                realWeight = FW_BOLD;
            }
            break;
        case FontEffects::weightBold: 
            weight = 1;
            realWeight = FW_BOLD;
            break;
        case FontEffects::weightBlack:
            weight = 2;            
            realWeight = FW_EXTRABOLD;
            break;
    };

    if (effects.subscript()||effects.superscript())
        height = height * 3 / 4;

    if (effects.isSmall())
        height = height * 9 / 10;
        
    
    if(NULL==fontsHandles_[height-6][weight][underline][strike])
        createFont(height, weight, realWeight, underline, strike);

    return fontsHandles_[height-6][weight][underline][strike];
}
