#include "ListNumberElement.hpp"

using ArsLexis::String;
using ArsLexis::char_t;

ListNumberElement::ListNumberElement(uint_t number):
    number_(number),
    childIndentation_(0)
{}

void ListNumberElement::calculateLayout(LayoutContext& layoutContext)
{
    static const uint_t bufferSize=16;    
    char_t buffer[bufferSize];
    // Yeah, everybody know I should use std::stringstream here... But it enlarges code by 12kB and gives a load of warnings of non-inlined functions.
    uint_t wideNumber=8;
    if (totalCount_>=10 && totalCount_<100)
        wideNumber=88;
    else if (totalCount_>=100 && totalCount_<1000)
        wideNumber=888;
    else if (totalCount_>=1000 && totalCount_<10000)
        wideNumber=8888;

    // This probably means that parser screwed something and didn't set totalCount_ properly.
    assert(totalCount_<10000);

    // Assuming we have properly set totalCount_, we'll get all the ListNumbers indented the same.
    tprintf(buffer, _T("%hd. "), wideNumber);
    String str(buffer);
    swapText(str);

    uint_t widthBefore=indentation()+layoutContext.usedWidth;
    TextElement::calculateLayout(layoutContext);
    childIndentation_=layoutContext.usedWidth-widthBefore;

    tprintf(buffer, _T("%hd. "), number_);
    str=buffer;
    swapText(str);
}

ListNumberElement::~ListNumberElement()
{}

void ListNumberElement::render(RenderingContext& rc) 
{
    // Prevent number from being displayed in reverse video while selected.
    uint_t selStart = rc.selectionStart;
    uint_t selEnd = rc.selectionEnd;
    rc.selectionStart = rc.selectionEnd = LayoutContext::progressCompleted;
    TextElement::render(rc);
    rc.selectionStart = selStart;
    rc.selectionEnd = selEnd;
}

void ListNumberElement::wordAtIndex(LayoutContext& lc, uint_t index, uint_t& wordStart, uint_t& wordEnd)
{
    wordStart = wordEnd = offsetOutsideElement;
}
