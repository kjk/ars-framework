#ifndef __ARSLEXIS_DEFINITION_STYLE_HPP__
#define __ARSLEXIS_DEFINITION_STYLE_HPP__

#if defined(_PALM_OS)
# include <PalmOS/PalmDefinitionStyle.hpp>
#else
# error "Define DefinitionStyle for your build target."
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

class DefinitionStyle;

uint_t getStaticStyleCount();

const char* getStaticStyleName(uint_t index);

const DefinitionStyle* getStaticStyle(uint_t index);

const DefinitionStyle* getStaticStyle(const char* name, uint_t length);

DefinitionStyle* parseStyle(const char* style, ulong_t length);

#endif
