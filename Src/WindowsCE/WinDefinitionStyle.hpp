#ifndef __ARSLEXIS_PALM_DEFINITION_STYLE_HPP__
#define __ARSLEXIS_PALM_DEFINITION_STYLE_HPP__

#include <BaseTypes.hpp>

typedef int FontID;

class DefinitionStyle
{
public:
    enum TriState
    {
        no,
        yes,
        notDefined
    };

    COLORREF        foregroundColor; // {-1,x,x,x} if not defined
    COLORREF        backgroundColor; // {-1,x,x,x} if not defined
    FontID              fontId; // -1 if not defined
    TriState            bold;
    TriState            italic;
    TriState            superscript;
    TriState            subscript;
    TriState            small;
    TriState            strike;
    UnderlineModeType   underline; // -1 if not defined
    
    void reset();
    
    DefinitionStyle& operator|=(const DefinitionStyle& other);
};

#ifdef DEBUG
void test_StaticStyleTable();
#endif

#endif
