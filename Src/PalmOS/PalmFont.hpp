#ifndef __ARSLEXIS_PALM_FONT_HPP__
#define __ARSLEXIS_PALM_FONT_HPP__

#include <PalmOS.h>
#include <BaseTypes.hpp>
#include <FontEffects.hpp>
#include <Utility.hpp>

static const FontID fontScalingDisabled = 0x40;
static const FontID tinyFont = FontID(stdFont | fontScalingDisabled);
static const FontID tinyBoldFont = FontID(boldFont | fontScalingDisabled);
static const FontID smallFont = FontID(largeFont | fontScalingDisabled);
static const FontID smallBoldFont = FontID(largeBoldFont | fontScalingDisabled);

class PalmFont
{

    FontID fontId_;
    FontEffects effects_;
        
public:

    PalmFont(FontID id = stdFont):
        fontId_(id)
    {}            
    
    FontID fontId() const
    {return fontId_;}
    
    FontEffects effects() const
    {return effects_;}       
    
    FontID withEffects() const;
    
    void addEffects(FontEffects effects)
    {effects_+=effects;}

    void setEffects(FontEffects effects)
    {effects_=effects;}
    
    void setFontId(FontID fontId)
    {fontId_=fontId;}
    
    static PalmFont getSymbolFont() 
    {return symbolFont;}

};

#endif