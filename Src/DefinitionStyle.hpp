#ifndef __ARSLEXIS_DEFINITION_STYLE_HPP__
#define __ARSLEXIS_DEFINITION_STYLE_HPP__

#include <Debug.hpp>

#if defined(_PALM_OS)
 #include <PalmOS/PalmDefinitionStyle.hpp>
#else
 // same as defined in <PalmOS.h>
 typedef enum 
 {
     noUnderline, 
     grayUnderline, 
     solidUnderline, 
     colorUnderline 
 } UnderlineModeType;
 #include <WindowsCE\WinDefinitionStyle.hpp>
 //#error "Define DefinitionStyle for your build target."
#endif

enum 
{
    styleIndexDefault = 0,
    styleIndexHyperlink = 1
};

#define styleNameDefault        ".default"
#define styleNameHyperlink      ".hyperlink"

#define styleNameBold           "bold"
#define styleNameLarge          "large"
#define styleNameBlue           "blue"
#define styleNameGray           "gray"
#define styleNameRed            "red"
#define styleNameGreen          "green"
#define styleNameYellow         "yellow"
#define styleNameBlack          "black"
#define styleNameBoldRed        "boldred"
#define styleNameBoldGreen      "boldgreen"
#define styleNameBoldBlue       "boldblue"
#define styleNameLargeBlue      "largeblue"
#define styleNameSmallGray      "smallgray"

#define styleNameHeader         "header"
#define styleNameSmallHeader    "smallheader"
#define styleNamePageTitle      "pagetitle"
#define styleNameStockPriceUp   "stockpriceup"
#define styleNameStockPriceDown "stockpricedown"

class DefinitionStyle;

uint_t getStaticStyleCount();

const char* getStaticStyleName(uint_t index);

const DefinitionStyle* getStaticStyle(uint_t index);

const DefinitionStyle* getStaticStyle(const char* name, uint_t length = uint_t(-1));

DefinitionStyle* StyleParse(const char* style, ulong_t length);

#define styleAttrNameFontFamily "font-family" // PalmOS: ignored
#define styleAttrValueFontFamilySerif "serif"
#define styleAttrValueFontFamilySansSerif "sans-serif"
#define styleAttrValueFontFamilyCursive "cursive"
#define styleAttrValueFontFamilyFantasy "fantasy"
#define styleAttrValueFontFamilyMonospace "monospace"

#define styleAttrNameFontStyle "font-style" // PalmOS: ignored
#define styleAttrValueFontStyleNormal "normal"
#define styleAttrValueFontStyleItalic "italic"
#define styleAttrValueFontStyleObligue "obligue"

#define styleAttrNameFontVariant "font-variant" // PalmOS: ignored
#define styleAttrValueFontVariantNormal "normal"
#define styleAttrValueFontVariantSmallCaps "small-caps"

#define styleAttrNameFontWeight "font-weight" // PalmOS: (weight <= normal) -> normal; (weight > normal) -> bold
#define styleAttrValueFontWeightNormal "normal"
#define styleAttrValueFontWeightLighter "lighter"
#define styleAttrValueFontWeightBold "bold"
#define styleAttrValueFontWeightBolder "bolder"

enum {
    styleAttrValueFontWeightNormalNum = 400,
    styleAttrValueFontWeightBoldNum = 700
};

// returns numeric font weight or -1 when value is invalid
long StyleParseFontWeight(const char* value, ulong_t length);
    
#define styleAttrNameFontSize "font-size" // PalmOS: (size <= medium) -> medium; (size > medium) -> large; medium == 10pt
#define styleAttrValueFontSizeXXSmall "xx-small"
#define styleAttrValueFontSizeXSmall "x-small"
#define styleAttrValueFontSizeSmall "small"
#define styleAttrValueFontSizeMedium "medium"
#define styleAttrValueFontSizeLarge "large"
#define styleAttrValueFontSizeXLarge "x-large"
#define styleAttrValueFontSizeXXLarge "xx-large"

enum {
    styleAttrValueFontSizeMediumNum = 10,
    styleAttrValueFontSizeLargeNum = 14
};

// returns numeric font size in points or -1 when value is invalid
long StyleParseFontSize(const char* value, ulong_t length);

#define styleAttrNameColor "color"
#define styleAttrNameBackgroundColor "background-color"

bool StyleParseColor(const char* value, ulong_t length, unsigned char& r, unsigned char& g, unsigned char& b);

#define styleAttrNameTextDecoration "text-decoration"
#define styleAttrValueTextDecorationNone "none"
#define styleAttrValueTextDecorationUnderline "underline"
#define styleAttrValueTextDecorationLineThrough "line-through"
#define styleAttrValueTextDecorationXUnderlineDotted "x-underline-dotted"

#define styleAttrNameVerticalAlign "vertical-align"
#define styleAttrValueVerticalAlignBaseline "baseline"
#define styleAttrValueVerticalAlignSubscript "sub"
#define styleAttrValueVerticalAlignSuperscript "superscript"

#ifndef NDEBUG
void test_StyleParse();
#endif

#endif
