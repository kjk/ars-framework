#ifndef __BULLET_ELEMENT_HPP__
#define __BULLET_ELEMENT_HPP__

#include "GenericTextElement.hpp"

class BulletElement: public GenericTextElement
{
    uint_t childIndentation_;
    
protected:

    uint_t childIndentation() const
    {return childIndentation_;}

public:
    
    
    BulletElement();

    bool breakBefore(const RenderingPreferences& preferences) const
    {return true;}
    
    void calculateLayout(LayoutContext& mc);
    
    virtual void toText(ArsLexis::String& appendTo, uint_t from, uint_t to) const
    {
        if (from!=to)
            appendTo.append(_T("\n• "));
    }
    
    void render(RenderingContext& rc);
    
    ~BulletElement();

};

#endif