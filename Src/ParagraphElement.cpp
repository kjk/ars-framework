#include "ParagraphElement.hpp"
#include "Utility.hpp"

void ParagraphElement::calculateOrRender(LayoutContext& layoutContext, Boolean render)
{
    assert(0==layoutContext.usedWidth);
    if (!render)
    {
        ArsLexis::FontSetter fontSet(stdFont);
        layoutContext.extendHeight(FntLineHeight(), FntBaseLine());
    }
    layoutContext.markElementCompleted(layoutContext.availableWidth());
}
