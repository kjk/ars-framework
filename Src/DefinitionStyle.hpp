#ifndef __ARSLEXIS_DEFINITION_STYLE_HPP__
#define __ARSLEXIS_DEFINITION_STYLE_HPP__

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

#define styleNameHeader         "header"
#define styleNameSmallHeader    "smallheader"
#define styleNamePageTitle      "pagetitle"
#define styleNameStockPriceUp   "stockpriceup"
#define styleNameStockPriceDown "stockpricedown"

class DefinitionStyle;

uint_t getStaticStyleCount();

const char* getStaticStyleName(uint_t index);

const DefinitionStyle* getStaticStyle(uint_t index);

const DefinitionStyle* getStaticStyle(const char* name, uint_t length = (uint_t)(-1));

DefinitionStyle* parseStyle(const char* style, ulong_t length);

#endif
