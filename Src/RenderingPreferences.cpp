#include "RenderingPreferences.hpp"

using ArsLexis::FontEffects;

RenderingPreferences::RenderingPreferences()
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
    FontEffects fx;
    fx.setUnderline(FontEffects::underlineDotted);
    for (uint_t i=0; i<hyperlinkTypesCount_; ++i) 
        hyperlinkDecorations_[i].font.setEffects(fx);
}
