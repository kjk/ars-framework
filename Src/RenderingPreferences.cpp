#include "RenderingPreferences.hpp"

RenderingPreferences::RenderingPreferences()
{
    styles_[styleHeader].fontId=largeFont;
    hyperlinkDecorations_[hyperlinkTerm].textColor=40;
    hyperlinkDecorations_[hyperlinkExternal].textColor=35;
}
