#include "FormattedTextElement.hpp"

using ArsLexis::Graphics;
using ArsLexis::String;
using ArsLexis::FontEffects;

FormattedTextElement::FormattedTextElement(const String& text):
    GenericTextElement(text)
{}

void FormattedTextElement::applyFormatting(Graphics& graphics, const RenderingPreferences& preferences)
{
    const RenderingPreferences::StyleFormatting& styleFormat=preferences.styleFormatting(style());
    ArsLexis::Font font=styleFormat.font;
    font.addEffects(fontEffects_);
    graphics.setFont(font);
    graphics.setTextColor(styleFormat.textColor);
    applyHyperlinkDecorations(graphics, preferences);
}
