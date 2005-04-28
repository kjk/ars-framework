#ifndef __ARSLEXIS_PALM_DEFINITION_STYLE_HPP__
#define __ARSLEXIS_PALM_DEFINITION_STYLE_HPP__

#include <BaseTypes.hpp>
#include "WinFont.h"

class DefinitionStyle
{
public:

	DefinitionStyle();

	~DefinitionStyle();

    enum TriState
    {
        no,
        yes,
        notDefined
    };

    COLORREF        foregroundColor; 
    COLORREF        backgroundColor;
	static const COLORREF colorNotDefined;

	enum FontSize {
		fontSizeNotDefined = -1,
		fontSizeNormal = 10
	};
	long fontSize;

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
		fontFamilyMonospace = FF_MODERN | FIXED_PITCH,
		fontFamilyNormal = fontFamilySansSerif
	};

	FontFamily fontFamily;

//	char_t fontFamilyCustomName[LF_FACESIZE];
	
//	bool fontFamilyCustom() const;
    
    void reset();
    
    DefinitionStyle& operator|=(const DefinitionStyle& other);

	const WinFont& font() const;

	void invalidateCachedFont() const;

private:

	mutable WinFont cachedFont_;

};

#ifdef DEBUG
void test_StaticStyleTable();
#endif

#endif
