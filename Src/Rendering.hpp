#include "Debug.hpp"
#include "BaseTypes.hpp"
#include "Graphics.hpp"

#define elementCompletedProgress ((uint_t)-1)

class RenderingPreferences;
class GenericTextElement;

/**
 * Used to exchange data during layout calculation stage.
 */
struct LayoutContext
{
    ArsLexis::Graphics& graphics;
    const RenderingPreferences& preferences;
    uint_t renderingProgress;
    const uint_t screenWidth;
    uint_t usedWidth;
    uint_t usedHeight;
    uint_t baseLine;
    const GenericTextElement* nextTextElement;
    bool breakTextOnLastWhitespace;
    
    LayoutContext(ArsLexis::Graphics& graph, const RenderingPreferences& prefs, uint_t theScreenWidth):
        graphics(graph),
        preferences(prefs),
        renderingProgress(0),
        screenWidth(theScreenWidth),
        usedWidth(0),
        usedHeight(0),
        baseLine(0),
        nextTextElement(0),
        breakTextOnLastWhitespace(false)
    {}
    
    void markElementCompleted(uint_t width)
    {
        renderingProgress=elementCompletedProgress;
        usedWidth+=width;
    }
    
    bool isElementCompleted() const
    {return (renderingProgress==elementCompletedProgress);}
    
    bool isFirstInLine() const
    {return (usedWidth==0);}
    
    void startNewLine()
    {
        usedWidth=0;
        usedHeight=0;
        baseLine=0;
    }
    
    uint_t availableWidth() const
    {return (usedWidth>screenWidth?0:screenWidth-usedWidth);}
    
    void extendHeight(uint_t reqHeight, uint_t reqBaseLine);
    
};

class Definition;

struct RenderingContext: public LayoutContext
{
    Definition& definition;
    const uint_t left;
    uint_t top;
    
    RenderingContext(ArsLexis::Graphics& graph, const RenderingPreferences& prefs, Definition& def, uint_t x, uint_t y, uint_t width):
        LayoutContext(graph, prefs, width),
        definition(def),
        left(x),
        top(y)
    {}
};
