#include "ParagraphElement.hpp"

void IndentedParagraphElement::calculateLayout(LayoutContext& layoutContext)
{
    setChildIndentation(layoutContext.preferences.standardIndentation());    ParagraphElement::calculateLayout(layoutContext);
}
