#include "RenderingPreferences.hpp"

using ArsLexis::FontEffects;
using ArsLexis::Graphics;

RenderingPreferences::RenderingPreferences():
    standardIndentation_(16)
{
    styles_[styleHeader].font=largeFont;
    UInt32 screenDepths=0;
    Boolean color=false;
    Err error=WinScreenMode(winScreenModeGet, NULL, NULL, &screenDepths, &color);
    if (screenDepths>=8) // 16+ colors
    {
        hyperlinkDecorations_[hyperlinkTerm].textColor=40;
        hyperlinkDecorations_[hyperlinkExternal].textColor=35;
    }
    else if (screenDepths>=2) // 4 colors
    {
        hyperlinkDecorations_[hyperlinkTerm].textColor=2; // Dark gray
        hyperlinkDecorations_[hyperlinkExternal].textColor=1; // Light gray
    }
    if (screenDepths>=2)
    {
        for (uint_t i=0; i<stylesCount_; ++i)
            styles_[i].textColor=UIColorGetTableEntryIndex(UIObjectForeground);        
    }
    FontEffects fx;
    fx.setUnderline(FontEffects::underlineDotted);
    for (uint_t i=0; i<hyperlinkTypesCount_; ++i) 
        hyperlinkDecorations_[i].font.setEffects(fx);
    
    Graphics::Font_t font(symbolFont);
    char bullet[3];
    bullet[0]=(bulletType()==bulletCircle)?symbolShiftPunc:symbolDiamondChr;
    bullet[1]=symbolShiftNone;
    bullet[2]=chrNull;
    Graphics graphics;
    Graphics::FontSetter setFont(graphics, font);
    standardIndentation_=graphics.textWidth(bullet, 2);
}
