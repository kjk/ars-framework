#include "ListNumberElement.hpp"

using ArsLexis::String;
using ArsLexis::char_t;

#if defined(__MWERKS__)
//# pragma inline_depth(100)
//# pragma inline_bottom_up on
#endif

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
    else if (totalCount_>1000) // This probably means that parser screwed something and didn't set totalCount_ properly.
        assert(false);

    // Assuming we have properly set totalCount_, we'll get all the ListNumbers indented the same.
    tprintf(buffer, _T("%hd. "), wideNumber);
    String str(buffer);
    swapText(str);

    uint_t widthBefore=indentation()+layoutContext.usedWidth;
    GenericTextElement::calculateLayout(layoutContext);
    childIndentation_=layoutContext.usedWidth-widthBefore;

    tprintf(buffer, _T("%hd. "), number_);
    str=buffer;
    swapText(str);
}