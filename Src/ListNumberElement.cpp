#include "ListNumberElement.hpp"

using ArsLexis::String;

ListNumberElement::ListNumberElement(UInt16 number):
    number_(number),
    childIndentation_(0)
{}

void ListNumberElement::calculateLayout(LayoutContext& layoutContext)
{
    static const UInt16 bufferSize=16;    
    char buffer[bufferSize];
    // Yeah, everybody know I should use std::stringstream here... But it enlarges code by 12kB and gives a load of warnings of non-inlined functions.
    UInt16 wideNumber=8;
    if (totalCount_>=10 && totalCount_<100)
        wideNumber=88;
    else if (totalCount_>=100 && totalCount_<1000)
        wideNumber=888;
    else if (totalCount_>1000) // This probably means that parser screwed something and didn't set totalCount_ properly.
        assert(false);

    // Assuming we have properly set totalCount_, we'll get all the ListNumbers indented the same.
    StrPrintF(buffer, "%hd. ", wideNumber);
    String str(buffer);
    swapText(str);

    Coord widthBefore=indentation()+layoutContext.usedWidth;
    GenericTextElement::calculateLayout(layoutContext);
    childIndentation_=layoutContext.usedWidth-widthBefore;

    StrPrintF(buffer, "%hd. ", number_);
    str=buffer;
    swapText(str);
}