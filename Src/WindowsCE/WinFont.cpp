#include <BaseTypes.hpp>
#include <Debug.hpp>

#include "WinFont.h"

using namespace ArsLexis;

typedef struct FontCacheEntry_ {
    HFONT fntHandle_;
    int   dy_;
    LONG   weight_;
    bool  fUnderline_;
    bool  fStrike_;
} FontCacheEntry_t;

#define MAX_FONT_CACHE_ENTRIES 20

FontCacheEntry_t g_fontCache[MAX_FONT_CACHE_ENTRIES];
static int g_cacheEntriesUsed = 0;

static HFONT FindFont(int dy, LONG weight, bool fUnderline, bool fStrike)
{
    for (int i=0; i<g_cacheEntriesUsed; i++)
    {
        if ( (dy==g_fontCache[i].dy_) &&
             (weight==g_fontCache[i].weight_) &&
             (fUnderline==g_fontCache[i].fUnderline_) &&
             (fStrike==g_fontCache[i].fStrike_) )
        {
            return g_fontCache[i].fntHandle_;
        }
    }
    return NULL;
}

static void AddCacheEntry(HFONT fntHandle, int dy, LONG weight, bool fUnderline, bool fStrike)
{
    assert(NULL==FindFont(dy,weight,fUnderline,fStrike));
    if (g_cacheEntriesUsed==MAX_FONT_CACHE_ENTRIES)
        return;
    g_fontCache[g_cacheEntriesUsed].fntHandle_ = fntHandle;
    g_fontCache[g_cacheEntriesUsed].dy_ = dy;
    g_fontCache[g_cacheEntriesUsed].weight_ = weight;
    g_fontCache[g_cacheEntriesUsed].fUnderline_ = fUnderline;
    g_fontCache[g_cacheEntriesUsed].fStrike_ = fStrike;
    g_cacheEntriesUsed++;
}

static HFONT createFont(int dy, LONG weight, bool fUnderline, bool fStrike)
{
    assert(dy>=4);
    HFONT fnt = FindFont(dy, weight, fUnderline, fStrike);
    if (NULL!=fnt)
        return fnt;

    LOGFONT logfnt;
    HFONT   stdFont = (HFONT)GetStockObject(SYSTEM_FONT);
    GetObject(stdFont, sizeof(logfnt), &logfnt);

    logfnt.lfWeight = weight;
    logfnt.lfHeight = -dy;

    if (fUnderline)
        logfnt.lfUnderline = TRUE;
    else
        logfnt.lfUnderline = FALSE;

    if (fStrike)
        logfnt.lfStrikeOut = TRUE;
    else
        logfnt.lfStrikeOut = FALSE;

    HFONT newFont = CreateFontIndirect(&logfnt);

    if (newFont)
    {
        AddCacheEntry(newFont, dy, weight, fUnderline, fStrike);
        return newFont;
    }
    else
        return stdFont;
}

HFONT getFont(int height, FontEffects effects)
{
    LONG weight = FW_NORMAL;

    bool fUnderline = false;
    if (effects.underline() != FontEffects::underlineNone)
        fUnderline = true;

    bool fStrike = false;
    if (effects.strikeOut())
        fStrike = true;

    switch (effects.weight())
    {
        case FontEffects::weightPlain:
            if (effects.italic())
            {
                weight = FW_BOLD;
            }
            break;
        case FontEffects::weightBold: 
            weight = FW_BOLD;
            break;
        case FontEffects::weightBlack:
            weight = FW_EXTRABOLD;
            break;
    };

    if (effects.subscript() || effects.superscript())
        height = height * 3 / 4;

    if (effects.isSmall())
        height = height * 9 / 10;

    return createFont(height, weight, fUnderline, fStrike);
}
