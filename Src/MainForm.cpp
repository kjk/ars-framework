#include "MainForm.hpp"
#include "GenericTextElement.hpp"

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
    rect.topLeft().x+=2;
    rect.topLeft().y+=20;
    rect.extent().x-=10;
    rect.extent().y-=40;
    //! @todo obtain RenderingPreferences from Application object and do the following right way...
    Definition::RenderingPreferences* prefs=0;
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
        definition_.appendElement(new GenericTextElement("To jest testowy element tekstowy. "));
        definition_.appendElement(new GenericTextElement("Ciekawe jak bedzie wygladal jego rendering. To jest bardzo dlugi wyraz: ", largeFont, grayUnderline));
        definition_.appendElement(new GenericTextElement("qwertyuioopasdfghhjklzxcvbnm,m,qwertr1234345556777800qwqwweqweqweqwrwerertredfsadfasdaa. On ma byc pociety podczas wyswietlania, bo inaczej sie nie zmiesci. Sprawdzimy wyswietlanie innych znakow: test. ", boldFont));
        definition_.appendElement(new GenericTextElement("For large UNIX projects, the traditional method of building the project is to use recursive "
"make. On some projects, this results in build times which are unacceptably large, when "
"all you want to do is change one file. In examining the source of the overly long build "
"times, it became evident that a number of apparently unrelated problems combine to produce "
"the delay, but on analysis all have the same root cause.\n"
"This paper explores a number of problems regarding the use of recursive make, and "
"shows that they are all symptoms of the same problem. Symptoms that the UNIX community "
"have long accepted as a fact of life, but which need not be endured any longer.\n"
"These problems include recursive makes which take ‘‘forever’’ to work out that they need "
"to do nothing, recursive makes which do too much, or too little, recursive makes which "
"are overly sensitive to changes in the source code and require constant Makefile intervention "
"to keep them working.\n"
"The resolution of these problems can be found by looking at what make does, from first "
"principles, and then analyzing the effects of introducing recursive make to this activity. "
"The analysis shows that the problem stems from the artificial partitioning of the build into "
"separate subsets. This, in turn, leads to the symptoms described. To avoid the symptoms, "
"it is only necessary to avoid the separation; to use a single make session to build the "
"whole project, which is not quite the same as a single Makefile.\n"
"This conclusion runs counter to much accumulated folk wisdom in building large projects "
"on UNIX. Some of the main objections raised by this folk wisdom are examined and "
"shown to be unfounded. The results of actual use are far more encouraging, with routine "
"development performance improvements significantly faster than intuition may indicate, "
"and without the intuitvely expected compromise of modularity. The use of a whole "
"project make is not as difficult to put into practice as it may at first appear."));
    }
    return error;
}