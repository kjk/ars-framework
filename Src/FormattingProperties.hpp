#ifndef __FORMATTING_PROPERTIES_HPP__
#define __FORMATTING_PROPERTIES_HPP__

#include "Debug.hpp"
#include "BaseTypes.hpp"

class FormattingProperties
{
    uint_t strength_:2;
    uint_t typewriter_:1;
    uint_t small_:1;
    uint_t strike_:1;
    uint_t underline_:1;

public:

    FormattingProperties();

    enum Strength
    {
        strengthPlain,
        strengthEmphasize,
        strengthStrong,
        strengthVeryStrong
    };

    void setStrength(Strength strength)
    {strength_=strength;}
    
    Strength strength() const
    {return static_cast<Strength>(strength_);}
    
    bool typewriterFont() const
    {return typewriter_;}

    void setTypewriterFont(bool val) 
    {typewriter_=val;}

    bool small() const
    {return small_;}
    
    void setSmall(bool val)
    {small_=val;}
    
    bool strikeOut() const
    {return strike_;}
    
    void setStrikeOut(bool val)
    {strike_=val;}
    
    bool underline() const
    {return underline_;}
    
   void setUnderline(bool val)
   {underline_=val;}

};

#endif