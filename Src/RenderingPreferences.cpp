#include "RenderingPreferences.hpp"
#include <PrefsStore.hpp>

using ArsLexis::FontEffects;
using ArsLexis::Graphics;
using ArsLexis::Font;

RenderingPreferences::RenderingPreferences():
    standardIndentation_(16),
    bulletIndentation_(2),
    backgroundColor_(0)
{
    styles_[styleHeader].font=largeFont;
    UInt32 screenDepths=0;
    Boolean color=false;
    Err error=WinScreenMode(winScreenModeGet, NULL, NULL, &screenDepths, &color);
    if (screenDepths>=8) // 16+ colors
    {
        hyperlinkDecorations_[hyperlinkTerm].textColor=57;
        hyperlinkDecorations_[hyperlinkExternal].textColor=35;
    }
    else if (screenDepths>=2) // 4 colors
    {
        hyperlinkDecorations_[hyperlinkTerm].textColor=2; // Dark gray
        hyperlinkDecorations_[hyperlinkExternal].textColor=1; // Light gray
    }
    if (screenDepths>=2)
    {
        for (uint_t i=0; i<stylesCount_; ++i)
            styles_[i].textColor=UIColorGetTableEntryIndex(UIObjectForeground);        
    }
    FontEffects fx;
    fx.setUnderline(FontEffects::underlineDotted);
    for (uint_t i=0; i<hyperlinkTypesCount_; ++i) 
        hyperlinkDecorations_[i].font.setEffects(fx);
    calculateIndentation();
}

void RenderingPreferences::calculateIndentation()
{
    const char* bullet="\x95";
    Graphics graphics;
    standardIndentation_=graphics.textWidth(bullet, 1)+bulletIndentation_;
}

Err RenderingPreferences::serializeOut(ArsLexis::PrefsStoreWriter& writer, int uniqueId) const
{
    Err error=errNone;
    if (errNone!=(error=writer.ErrSetUInt16(uniqueId++, backgroundColor_)))
        goto OnError;
    if (errNone!=(error=writer.ErrSetUInt16(uniqueId++, bulletIndentation_)))
        goto OnError;
    for (uint_t i=0; i<hyperlinkTypesCount_; ++i)
    {
        if (errNone!=(error=hyperlinkDecorations_[i].serializeOut(writer, uniqueId)))
            goto OnError;
        uniqueId+=StyleFormatting::reservedPrefIdCount;
    }
    for (uint_t i=0; i<stylesCount_; ++i)
    {
        if (errNone!=(error=styles_[i].serializeOut(writer, uniqueId)))
            goto OnError;
        uniqueId+=StyleFormatting::reservedPrefIdCount;
    }
OnError:
    return error;    
}

Err RenderingPreferences::serializeIn(ArsLexis::PrefsStoreReader& reader, int uniqueId)
{
    Err error=errNone;
    RenderingPreferences tmp;
    UInt16 val;
    if (errNone!=(error=reader.ErrGetUInt16(uniqueId++, &val)))
        goto OnError;
    tmp.setBackgroundColor(val);        
    if (errNone!=(error=reader.ErrGetUInt16(uniqueId++, &val)))
        goto OnError;
    bulletIndentation_=val;
    for (uint_t i=0; i<hyperlinkTypesCount_; ++i)
    {
        if (errNone!=(error=tmp.hyperlinkDecorations_[i].serializeIn(reader, uniqueId)))
            goto OnError;
        uniqueId+=StyleFormatting::reservedPrefIdCount;
    }
    for (uint_t i=0; i<stylesCount_; ++i)
    {
        if (errNone!=(error=tmp.styles_[i].serializeIn(reader, uniqueId)))
            goto OnError;
        uniqueId+=StyleFormatting::reservedPrefIdCount;
    }
    (*this)=tmp;
OnError:
    return error;    
}

Err RenderingPreferences::StyleFormatting::serializeOut(ArsLexis::PrefsStoreWriter& writer, int uniqueId) const
{
    Err error=errNone;
    if (errNone!=(error=writer.ErrSetUInt16(uniqueId++, font.fontId())))
        goto OnError;
    if (errNone!=(error=writer.ErrSetUInt16(uniqueId++, font.effects().mask())))
        goto OnError;
    if (errNone!=(error=writer.ErrSetUInt16(uniqueId++, textColor)))
        goto OnError;
OnError:
    return error;
}

Err RenderingPreferences::StyleFormatting::serializeIn(ArsLexis::PrefsStoreReader& reader, int uniqueId)
{
    Err error=errNone;
    StyleFormatting tmp;
    UInt16 val;
    if (errNone!=(error=reader.ErrGetUInt16(uniqueId++, &val)))
        goto OnError;
    tmp.font.setFontId(static_cast<FontID>(val));
    if (errNone!=(error=reader.ErrGetUInt16(uniqueId++, &val)))
        goto OnError;
    tmp.font.setEffects(ArsLexis::FontEffects(val));
    if (errNone!=(error=reader.ErrGetUInt16(uniqueId++, &val)))
        goto OnError;
    tmp.textColor=val;
    (*this)=tmp;
OnError:    
    return error;
}
