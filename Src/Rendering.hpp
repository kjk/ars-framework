#include "Debug.hpp"

#define elementCompletedProgress ((UInt16)-1)

class RenderingPreferences;

/**
 * Used to exchange data during layout calculation stage.
 */
struct LayoutContext
{
    const RenderingPreferences& preferences;
    UInt16 renderingProgress;
    const Coord screenWidth;
    Coord usedWidth;
    Coord usedHeight;
    Coord baseLine;
    
    LayoutContext(const RenderingPreferences& prefs, Coord theScreenWidth):
        preferences(prefs),
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

class Definition;

struct RenderingContext: public LayoutContext
{
    Definition& definition;
    const Coord left;
    Coord top;
    
    RenderingContext(const RenderingPreferences& prefs, Definition& def, Coord x, Coord y, Coord width):
        LayoutContext(prefs, width),
        definition(def),
        left(x),
        top(y)
    {}
};
