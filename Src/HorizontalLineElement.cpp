#include "HorizontalLineElement.hpp"

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
    WinDrawLine(left+2, top, renderingContext.availableWidth()-4, top);
    renderingContext.markElementCompleted(renderingContext.availableWidth());
}
    
