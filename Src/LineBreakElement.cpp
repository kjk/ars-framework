#include "LineBreakElement.hpp"
#include "Graphics.hpp"

using ArsLexis::Graphics;

void LineBreakElement::calculateOrRender(LayoutContext& layoutContext, bool render)
{
    assert(0==layoutContext.usedWidth);
    if (!render)
    {
        const RenderingPreferences::StyleFormatting& style=layoutContext.preferences.styleFormatting(styleDefault);
        Graphics::FontSetter setFont(layoutContext.graphics, style.fontId);
        layoutContext.extendHeight(FntLineHeight(), FntBaseLine());
    }
    layoutContext.markElementCompleted(0);
}
