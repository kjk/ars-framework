#include "FormattedTextElement.hpp"

using ArsLexis::Graphics;
using ArsLexis::String;

FormattedTextElement::FormattedTextElement(const String& text):
    GenericTextElement(text)
{}

void FormattedTextElement::applyFormatting(Graphics& graphics, const RenderingPreferences& preferences)
{
    GenericTextElement::applyFormatting(graphics, preferences);
    const RenderingPreferences::StyleFormatting& styleFormat=preferences.styleFormatting(style());
    if (formatting_.strength()!=FormattingProperties::strengthPlain)
    {
        Graphics::Font_t originalFont=styleFormat.font;
        graphics.setFont(graphics.makeBold(originalFont));
    }
    if (formatting_.underline())
        WinSetUnderlineMode(preferences.standardUnderline());
}
