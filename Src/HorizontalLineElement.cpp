#include "HorizontalLineElement.hpp"
#include "Utility.hpp"

void HorizontalLineElement::calculateLayout(LayoutContext& layoutContext)
{
    layoutContext.extendHeight(5, 3);
    layoutContext.markElementCompleted(layoutContext.availableWidth());
}

void HorizontalLineElement::render(RenderingContext& renderingContext)
{
    Coord indent=indentation();
    if (renderingContext.usedWidth<indent)
        renderingContext.usedWidth=indent;
    Coord top=renderingContext.top+renderingContext.usedHeight/2;
    Coord left=renderingContext.left+renderingContext.usedWidth;
    
    const RenderingPreferences::StyleFormatting& format=renderingContext.preferences.styleFormatting(styleDefault);
    ArsLexis::ForegroundColorSetter setForeColor(format.textColor);
    WinPaintLine(left+2, top, left+renderingContext.availableWidth()-2, top);
    
    renderingContext.markElementCompleted(renderingContext.availableWidth());
}
    
