#include <Debug.hpp>
#include <BaseTypes.hpp>
#include <Graphics.hpp>

class TextElement;

/**
 * Used to exchange data during layout calculation stage.
 */
struct LayoutContext
{

    enum {progressCompleted=((uint_t)-1)};


    Graphics& graphics;
    uint_t renderingProgress;
    const uint_t screenWidth;
    uint_t usedWidth;
    uint_t usedHeight;
    uint_t baseLine;
    TextElement* nextTextElement;
    uint_t selectionStart;
    uint_t selectionEnd;
    bool selectionIsHyperlink;
    
    LayoutContext(Graphics& graph, uint_t theScreenWidth):
        graphics(graph),
        renderingProgress(0),
        screenWidth(theScreenWidth),
        usedWidth(0),
        usedHeight(0),
        baseLine(0),
        nextTextElement(0),
        selectionStart(progressCompleted),
        selectionEnd(progressCompleted),
        selectionIsHyperlink(false)
    {}
    
    void markElementCompleted(uint_t width)
    {
        renderingProgress = progressCompleted;
        usedWidth += width;
    }
    
    bool isElementCompleted() const
    {return (renderingProgress==progressCompleted);}
    
    bool isFirstInLine() const
    {return (usedWidth==0);}
    
    void startNewLine()
    {
        usedWidth = 0;
        usedHeight = 0;
        baseLine = 0;
    }
    
    uint_t availableWidth() const
    {return (usedWidth>screenWidth?0:screenWidth-usedWidth);}
    
    void extendHeight(uint_t reqHeight, uint_t reqBaseLine);
    
};

class Definition;

struct RenderingContext: public LayoutContext
{
    Definition& definition;
    uint_t left;
    uint_t top;
    
    RenderingContext(Graphics& graph, Definition& def, uint_t x, uint_t y, uint_t width):
        LayoutContext(graph, width),
        definition(def),
        left(x),
        top(y)
    {}
};
