#include "MainForm.hpp"
#include "GenericTextElement.hpp"
#include "BulletElement.hpp"
#include "ParagraphElement.hpp"
#include "HorizontalLineElement.hpp"

void MainForm::resize(const RectangleType& screenBounds)
{
    setBounds(screenBounds);

    RectangleType bounds;
    UInt16 index=getObjectIndex(definitionScrollBar);
    getObjectBounds(index, bounds);
    bounds.topLeft.x=screenBounds.extent.x-8;
    bounds.extent.y=screenBounds.extent.y-36;
    setObjectBounds(index, bounds);
    
    index=getObjectIndex(termLabel);
    getObjectBounds(index, bounds);
    bounds.topLeft.y=screenBounds.extent.y-14;
    setObjectBounds(index, bounds);

    index=getObjectIndex(termInputField);
    getObjectBounds(index, bounds);
    bounds.topLeft.y=screenBounds.extent.y-14;
    bounds.extent.x=screenBounds.extent.x-60;
    setObjectBounds(index, bounds);

    index=getObjectIndex(goButton);
    getObjectBounds(index, bounds);
    bounds.topLeft.x=screenBounds.extent.x-26;
    bounds.topLeft.y=screenBounds.extent.y-14;
    setObjectBounds(index, bounds);

    update();    
}

void MainForm::draw(UInt16 updateCode)
{
    iPediaForm::draw(updateCode);
    ArsLexis::Rectangle rect=bounds();
    WinPaintLine(rect.x(), rect.height()-18, rect.width(), rect.height()-18);

    rect.explode(2, 18, -12, -36);
    //! @todo Obtain RenderingPreferences from Application object and do the following right way...
    RenderingPreferences* prefs=0;
    definition_.render(rect, *prefs);
    UInt16 index=getObjectIndex(definitionScrollBar);
    ScrollBarType* scrollBar=static_cast<ScrollBarType*>(FrmGetObjectPtr(*this, index));
    SclSetScrollBar(scrollBar, definition_.firstShownLine(), 0, definition_.totalLinesCount()-definition_.shownLinesCount(), definition_.shownLinesCount());
}


Err MainForm::initialize()
{
    Err error=iPediaForm::initialize();
    if (!error)
    {
        DefinitionElement* element=0;
        definition_.appendElement(element=new GenericTextElement(
            "For large UNIX projects, the traditional method of building the project is to use recursive "
            "make. On some projects, this results in build times which are unacceptably large, when "
            "all you want to do is change one file. In examining the source of the overly long build "
            "times, it became evident that a number of apparently unrelated problems combine to produce "
            "the delay, but on analysis all have the same root cause. "
        ));
        definition_.appendElement(new ParagraphElement());
        DefinitionElement* bullet=0;
        definition_.appendElement(bullet=new BulletElement());       
        definition_.appendElement(element=new GenericTextElement(
            "This paper explores a number of problems regarding the use of recursive make, and "
            "shows that they are all symptoms of the same problem. Symptoms that the UNIX community "
            "have long accepted as a fact of life, but which need not be endured any longer. "
            "These problems include recursive makes which take ‘‘forever’’ to work out that they need "
            "to do nothing, recursive makes which do too much, or too little, recursive makes which "
            "are overly sensitive to changes in the source code and require constant Makefile intervention "
            "to keep them working. "
        ));
        element->setParent(*bullet);
        definition_.appendElement(new ParagraphElement());
        definition_.appendElement(new GenericTextElement(
            "The resolution of these problems can be found by looking at what make does, from first "
            "principles, and then analyzing the effects of introducing recursive make to this activity. "
            "The analysis shows that the problem stems from the artificial partitioning of the build into "
            "separate subsets. This, in turn, leads to the symptoms described. To avoid the symptoms, "
            "it is only necessary to avoid the separation; to use a single make session to build the "
            "whole project, which is not quite the same as a single Makefile. "
        ));
        definition_.appendElement(new HorizontalLineElement());
        definition_.appendElement(new GenericTextElement(
            "This conclusion runs counter to much accumulated folk wisdom in building large projects "
            "on UNIX. Some of the main objections raised by this folk wisdom are examined and "
            "shown to be unfounded. The results of actual use are far more encouraging, with routine "
            "development performance improvements significantly faster than intuition may indicate, "
            "and without the intuitvely expected compromise of modularity. The use of a whole "
            "project make is not as difficult to put into practice as it may at first appear. "
        ));
    }
    return error;
}

inline void MainForm::handleScrollRepeat(const sclRepeat& data)
{
    definition_.scroll(data.newValue-data.value);
}

void MainForm::handlePenUp(const EventType& event)
{
    PointType point;
    point.x=event.screenX;
    point.y=event.screenY;
    if (definition_.bounds() && point)
        definition_.hitTest(point); 
}


Boolean MainForm::handleEvent(EventType& event)
{
    Boolean handled=false;
    switch (event.eType)
    {
        case penUpEvent:
            handlePenUp(event);
            break;
            
        case sclRepeatEvent:
            handleScrollRepeat(event.data.sclRepeat);
            break;
    
        default:
            handled=iPediaForm::handleEvent(event);
    }
    return handled;
}