#ifndef __ARSLEXIS_FONT_EFFECTS_HPP__
#define __ARSLEXIS_FONT_EFFECTS_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>

namespace ArsLexis
{

    class FontEffects
    {
        union
        {   
            struct 
            {
                uint_t weight:2;
                uint_t italic: 1;
                uint_t isSmall:1;
                uint_t strike:1;
                uint_t superscript:1;
                uint_t subscript:1;
                uint_t underline:2;
            } fx;
            uint_t mask;
        } effects_;
        
    public:

        FontEffects(uint_t mask=0)
        {effects_.mask=mask;}

        enum Weight
        {
            weightPlain,
            weightBold,
            weightBlack
        };
        
        enum Underline
        {
            underlineNone,
            underlineDotted,
            underlineSolid
        };

        void setWeight(Weight weight)
        {effects_.fx.weight=weight;}
        
        Weight weight() const
        {return static_cast<Weight>(effects_.fx.weight);}
        
        bool isSmall() const
        {return effects_.fx.isSmall;}
        
        void setSmall(bool val)
        {effects_.fx.isSmall=val;}
        
        bool strikeOut() const
        {return effects_.fx.strike;}
        
        void setStrikeOut(bool val)
        {effects_.fx.strike=val;}
        
        Underline underline() const
        {return static_cast<Underline>(effects_.fx.underline);}
        
       void setUnderline(Underline val)
       {effects_.fx.underline=val;}
       
       bool italic() const
       {return effects_.fx.italic;}
       
       void setItalic(bool val)
       {effects_.fx.italic=val;}

        bool superscript() const
        {return effects_.fx.superscript;}
        
        void setSuperscript(bool val)
        {effects_.fx.superscript=val;}
        
        bool subscript() const
        {return effects_.fx.subscript;}
        
        void setSubscript(bool val)
        {effects_.fx.subscript=val;}
        
        FontEffects& operator+=(const FontEffects& eff);
        
        bool empty() const
        {return (0==effects_.mask);}

        bool operator==(const FontEffects& eff) const
        {return effects_.mask==eff.effects_.mask;}

        bool operator!=(const FontEffects& eff) const
        {return effects_.mask!=eff.effects_.mask;}
        
        void clear()
        {effects_.mask=0;}
        
        uint_t mask() const
        {return effects_.mask;}
        
        void setMask(uint_t mask)
        {effects_.mask=mask;}

    };

}
#endif