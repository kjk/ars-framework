#include <BaseTypes.hpp>
#include <Debug.hpp>

#include "WinFont.h"

using namespace ArsLexis;

typedef struct FontCacheEntry_ {
    HFONT fntHandle;

} FontCacheEntry_t;

// cache of font handles
HFONT g_fontHandles[16-6][3][2][2];

static void InitFontHandles()
{
    static bool g_fInitialized = false;
    if (g_fInitialized)
        return;

    for (int a=0; a<16-6; a++)
    {
        for (int b=0; b<3; b++)
        {
            for (int c=0; c<2; c++)
            {
                for (int d=0; d<2; d++)
                {
                    g_fontHandles[a][b][c][d];
                }
            }
        }
    }
    g_fInitialized = true;
}

static HFONT createFont(int height, int weight, int realWeight, int isUnder, int isStrike)
{
    // InitFontHandles();

    assert( (height-6>=0) && (height-6<16-6) );
    assert( (weight>=0) && (weight<3) );
    assert( (isUnder>=0) && (isUnder<2) );
    assert( (isStrike>=0) && (isStrike<2) );

/*    if (NULL != g_fontHandles[height-6][weight][isUnder][isStrike])
        return g_fontHandles[height-6][weight][isUnder][isStrike];*/

    LOGFONT logfnt;
    HFONT   stdFont = (HFONT)GetStockObject(SYSTEM_FONT);
    GetObject(stdFont, sizeof(logfnt), &logfnt);

    logfnt.lfWeight = realWeight;
    logfnt.lfHeight = -height;

    if (isUnder)
        logfnt.lfUnderline = TRUE;
    if (isStrike)
        logfnt.lfStrikeOut = TRUE;
    assert(height<maxFontHeight);

    HFONT newFont = CreateFontIndirect(&logfnt);

    if (newFont)
        return newFont;
    else
        return stdFont;

/*    if (NULL == newFont)
        g_fontHandles[height-6][weight][isUnder][isStrike] = stdFont;
    else
        g_fontHandles[height-6][weight][isUnder][isStrike] = newFont;

    return g_fontHandles[height-6][weight][isUnder][isStrike]; */
}

HFONT getFont(int height, FontEffects effects)
{
    LONG realWeight = FW_NORMAL;

    int underline = 0;
    if (effects.underline() != FontEffects::underlineNone)
        underline = 1;

    int strike = 0;
    if (effects.strikeOut())
        strike = 1;

    int weight = 0;
    switch (effects.weight())
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

    if (effects.subscript() || effects.superscript())
        height = height * 3 / 4;

    if (effects.isSmall())
        height = height * 9 / 10;

    return createFont(height, weight, realWeight, underline, strike);
}
