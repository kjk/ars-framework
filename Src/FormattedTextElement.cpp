#include "FormattedTextElement.hpp"

using ArsLexis::String;

FormattedTextElement::FormattedTextElement(const String& text):
    GenericTextElement(text)
{}

void FormattedTextElement::applyFormatting(Graphics& graphics, const RenderingPreferences& preferences)
{
    graphics.applyStyle(*getStyle(), isHyperlink());
}

FormattedTextElement::~FormattedTextElement()
{}