#ifndef __ARSLEXIS_FONT_EFFECTS_HPP__
#define __ARSLEXIS_FONT_EFFECTS_HPP__

#include "Debug.hpp"
#include "BaseTypes.hpp"

namespace ArsLexis
{

    class FontEffects
    {
        uint_t weight_:2;
//        uint_t typewriter_:1;
        uint_t italic_: 1;
        uint_t small_:1;
        uint_t strike_:1;
        uint_t superscript_:1;
        uint_t subscript_:1;
        uint_t underline_:2;

    public:

        FontEffects();

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
        {weight_=weight;}
        
        Weight weight() const
        {return static_cast<Weight>(weight_);}
        
//        bool typewriterFont() const
//        {return typewriter_;}

//        void setTypewriterFont(bool val) 
//        {typewriter_=val;}

        bool small() const
        {return small_;}
        
        void setSmall(bool val)
        {small_=val;}
        
        bool strikeOut() const
        {return strike_;}
        
        void setStrikeOut(bool val)
        {strike_=val;}
        
        Underline underline() const
        {return static_cast<Underline>(underline_);}
        
       void setUnderline(Underline val)
       {underline_=val;}
       
       bool italic() const
       {return italic_;}
       
       void setItalic(bool val)
       {italic_=val;}

        bool superscript() const
        {return superscript_;}
        
        void setSuperscript(bool val)
        {superscript_=val;}
        
        bool subscript() const
        {return subscript_;}
        
        void setSubscript(bool val)
        {subscript_=val;}
        
        FontEffects& operator+=(const FontEffects& eff);

    };

}
#endif