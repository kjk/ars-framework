#include "LineBreakElement.hpp"
#include "Utility.hpp"

void LineBreakElement::calculateOrRender(LayoutContext& layoutContext, Boolean render)
{
    assert(0==layoutContext.usedWidth);
    if (!render)
    {
        ArsLexis::FontSetter setFont(stdFont);
        layoutContext.extendHeight(FntLineHeight(), FntBaseLine());
    }
    layoutContext.markElementCompleted(0);
}
