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
    
    void reset();
    
    DefinitionStyle& operator|=(const DefinitionStyle& other);
};


struct StaticStyleEntry
{
    const char* name;
    const DefinitionStyle style;
    
    // For use by binary search (std::lower_bound())
    bool operator<(const StaticStyleEntry& other) const
    {
        return tstrcmp(name, other.name) < 0;
    }

    bool operator!=(const StaticStyleEntry& other) const
    {
        return tstrcmp(name, other.name) != 0;
    }
    
};

#define COLOR_NOT_DEF_INDEX -1
#define COLOR_DEF_INDEX 0
#define COLOR_NOT_DEF_COLOR 0
#define COLOR_NOT_DEF  {COLOR_NOT_DEF_INDEX, COLOR_NOT_DEF_COLOR, COLOR_NOT_DEF_COLOR, COLOR_NOT_DEF_COLOR}
#define RGB(r,g,b)  {COLOR_DEF_INDEX,r,g,b}
#define WHITE    RGB(255,255,255)
#define BLACK    RGB(0,0,0)
#define GREEN    RGB(0,196,0)
#define BLUE     RGB(0,0,255)
#define RED      RGB(255,0,0)
#define YELLOW   RGB(255,255,0)
#define GRAY     RGB(127,127,127)
#define COLOR_UI_MENU_SELECTED_FILL   RGB(51,0,153)
#define COLOR_UI_FORM_FRAME           RGB(51,0,153)
#define FONT_NOT_DEF  FontID(-1)
#define NOT_DEF          DefinitionStyle::notDefined
#define TRUE            DefinitionStyle::yes
#define FALSE           DefinitionStyle::no
#define UNDERLINE_NOT_DEF UnderlineModeTag(-1)

#define COLOR(name, color) \
    {(name), {color, COLOR_NOT_DEF, FONT_NOT_DEF, NOT_DEF, NOT_DEF, NOT_DEF, NOT_DEF, NOT_DEF, NOT_DEF, UNDERLINE_NOT_DEF}}
#define COLOR_BOLD(name, color) \
    {(name), {color, COLOR_NOT_DEF, FONT_NOT_DEF, TRUE, NOT_DEF, NOT_DEF, NOT_DEF, NOT_DEF, NOT_DEF, UNDERLINE_NOT_DEF}}
#define COLOR_AND_FONT(name, color, font) \
    {(name), {color, COLOR_NOT_DEF, font, NOT_DEF, NOT_DEF, NOT_DEF, NOT_DEF, NOT_DEF, NOT_DEF, UNDERLINE_NOT_DEF}}
#define COLOR_AND_FONT_BOLD(name, color, font) \
    {(name), {color, COLOR_NOT_DEF, font, TRUE, NOT_DEF, NOT_DEF, NOT_DEF, NOT_DEF, NOT_DEF, UNDERLINE_NOT_DEF}}

const DefinitionStyle* StyleGetStaticStyleHelper(const StaticStyleEntry* array, uint_t arraySize, const char* name, uint_t length);


#ifdef DEBUG
void test_StaticStyleTable();
#endif

#endif