#ifndef __ARSLEXIS_PALM_DEFINITION_STYLE_HPP__
#define __ARSLEXIS_PALM_DEFINITION_STYLE_HPP__

#include <BaseTypes.hpp>
#include <PalmOS.h>

class DefinitionStyle
{
public:
    enum TriState
    {
        no,
        yes,
        notDefined
    };
    
    RGBColorType        foregroundColor; // {-1,x,x,x} if not defined
    RGBColorType        backgroundColor; // {-1,x,x,x} if not defined
    FontID              fontId; // -1 if not defined
    TriState            bold;
    TriState            italic;
    TriState            superscript;
    TriState            subscript;
    TriState            small;
    TriState            strike;
    UnderlineModeType   underline; // -1 if not defined
};

#ifdef DEBUG
void test_StaticStyleTable();
#endif

#endif