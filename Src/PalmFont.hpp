#ifndef __ARSLEXIS_PALM_FONT_HPP__
#define __ARSLEXIS_PALM_FONT_HPP__

#include <PalmOS.h>
#include <BaseTypes.hpp>
#include <FontEffects.hpp>
#include <Utility.hpp>

namespace ArsLexis
{

    class PalmFont
    {
    
        FontID fontId_;
        FontEffects effects_;
            
    public:
    
        PalmFont(FontID id=stdFont):
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

}

#endif