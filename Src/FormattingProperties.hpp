#ifndef __FORMATTING_PROPERTIES_HPP__
#define __FORMATTING_PROPERTIES_HPP__

#include <PalmOS.h>

class FormattingProperties
{
    UInt16 strength_:2;
    UInt16 typewriter_:1;
    UInt16 small_:1;
    UInt16 strike_:1;
    UInt16 underline_:1;

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
    
    Boolean typewriterFont() const
    {return typewriter_;}
    void setTypewriterFont(Boolean val) 
    {typewriter_=val;}

    Boolean small() const
    {return small_;}
    
    void setSmall(Boolean val)
    {small_=val;}
    
    Boolean strikeOut() const
    {return strike_;}
    
    void setStrikeOut(Boolean val)
    {strike_=val;}
    
    Boolean underline() const
    {return underline_;}
    
   void setUnderline(Boolean val)
   {underline_=val;}
};

#endif