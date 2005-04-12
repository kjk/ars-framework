#ifndef __ARSLEXIS_PALM_DEFINITION_STYLE_HPP__
#define __ARSLEXIS_PALM_DEFINITION_STYLE_HPP__

#include <BaseTypes.hpp>

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

	enum FontWeight {
		fontWeightNotDefined = -1,
		fontWeightNormal = 400,
		fontWeightBold = 700,
		fontWeightBlack = 900
	};
	long fontWeight;

    TriState            italic;
    TriState            superscript;
    TriState            subscript;
    TriState            small;
    TriState            strike;
	TriState			underline;
	
	enum FontFamily {
		fontFamilyNotDefined = 0,
		fontFamilySerif = FF_ROMAN | VARIABLE_PITCH,
		fontFamilySansSerif = FF_SWISS | VARIABLE_PITCH,
		fontFamilyCursive = FF_SCRIPT | VARIABLE_PITCH,
		fontFamilyFantasy = FF_DECORATIVE | VARIABLE_PITCH,
		fontFamilyMonospace = FF_MODERN | FIXED_PITCH
	};

	FontFamily fontFamily;
	char_t fontFamilyCustomName[LF_FACESIZE];
	
	bool fontFamilyCustom() const;
    
    void reset();
    
    DefinitionStyle& operator|=(const DefinitionStyle& other);
};

#ifdef DEBUG
void test_StaticStyleTable();
#endif

#endif
