#include "HorizontalLineElement.hpp"

using ArsLexis::Graphics;

void HorizontalLineElement::calculateLayout(LayoutContext& layoutContext)
{
    layoutContext.extendHeight(5, 3);
    layoutContext.markElementCompleted(layoutContext.availableWidth());
}

void HorizontalLineElement::render(RenderingContext& renderingContext)
{
    uint_t indent=indentation();
    if (renderingContext.usedWidth<indent)
        renderingContext.usedWidth=indent;
    uint_t top=renderingContext.top+renderingContext.usedHeight/2;
    uint_t left=renderingContext.left+renderingContext.usedWidth;
    
    const RenderingPreferences::StyleFormatting& format=renderingContext.preferences.styleFormatting(styleDefault);
    Graphics::ColorSetter setForeColor(renderingContext.graphics, Graphics::colorForeground, format.textColor);
    renderingContext.graphics.drawLine(left+2, top, left+renderingContext.availableWidth()-2, top);
    
    renderingContext.markElementCompleted(renderingContext.availableWidth());
}
    
HorizontalLineElement::~HorizontalLineElement()
{}

uint_t HorizontalLineElement::charIndexAtOffset(LayoutContext& lc, uint_t offset) {
    return 0;
}
