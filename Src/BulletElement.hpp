#ifndef __BULLET_ELEMENT_HPP__
#define __BULLET_ELEMENT_HPP__

#include "GenericTextElement.hpp"

class BulletElement: public GenericTextElement
{
    Coord childIndentation_;
    
protected:

    void applyFormatting(const RenderingPreferences& prefs);
    
    Coord childIndentation() const
    {return childIndentation_;}

public:
    
    
    BulletElement();

    Boolean requiresNewLine(const RenderingPreferences& preferences) const
    {return true;}
    
    void calculateLayout(LayoutContext& mc);

};

#endif