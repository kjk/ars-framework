#include "FormattedTextElement.hpp"
#include "Utility.hpp"

FormattedTextElement::FormattedTextElement(const ArsLexis::String& text):
    GenericTextElement(text)
{}

void FormattedTextElement::applyFormatting(const RenderingPreferences& preferences)
{
    GenericTextElement::applyFormatting(preferences);
    const RenderingPreferences::StyleFormatting& styleFormat=preferences.styleFormatting(style());
    if (formatting_.strength()!=FormattingProperties::strengthPlain)
    {
        FontID originalFontId=styleFormat.fontId;
        FntSetFont(ArsLexis::getBoldFont(originalFontId));
    }
    if (formatting_.underline())
        WinSetUnderlineMode(preferences.standardUnderline());
}
