#ifndef __DEFINITIONELEMENT_HPP__
#define __DEFINITIONELEMENT_HPP__

#include "Definition.hpp"

#define elementCompletedProgress ((UInt16)-1)

class DefinitionElement 
{
public:
    
    virtual Boolean requiresNewLine() const
    {return false;}
    
    /**
     * Used to exchange data during layout calculation stage.
     */
    struct LayoutContext
    {
        UInt16 renderingProgress;
        const Coord screenWidth;
        Coord usedWidth;
        Coord usedHeight;
        Coord baseLine;
        
        LayoutContext(Coord theScreenWidth):
            renderingProgress(0),
            screenWidth(theScreenWidth),
            usedWidth(0),
            usedHeight(0),
            baseLine(0)
        {}
        
        void markElementCompleted(Coord width)
        {
            renderingProgress=elementCompletedProgress;
            usedWidth+=width;
            assert(usedWidth<=screenWidth);
        }
        
        Boolean isElementCompleted() const
        {return (renderingProgress==elementCompletedProgress);}
        
        Boolean isFirstInLine() const
        {return (usedWidth==0);}
        
        void startNewLine()
        {
            usedWidth=0;
            usedHeight=0;
            baseLine=0;
        }
        
        Coord availableWidth() const
        {return (usedWidth>screenWidth?0:screenWidth-usedWidth);}
        
        void extendHeight(Coord reqHeight, Coord reqBaseLine);
        
    };
    
    struct RenderContext: public LayoutContext
    {
        Definition& definition;
        const Coord left;
        Coord top;
        const Coord topOffset;
        
        RenderContext(Definition& def, Coord x, Coord y, Coord tpOff, Coord width):
            LayoutContext(width),
            definition(def),
            left(x),
            top(y),
            topOffset(tpOff)
        {}
    };

    virtual void calculateLayout(LayoutContext& mc)=0;
    
    virtual void render(RenderContext& rc)=0;

    virtual ~DefinitionElement()
    {}
};

#endif