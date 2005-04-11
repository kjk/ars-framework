#include <DefinitionStyle.hpp>
#include <Graphics.hpp>
#include <Debug.hpp>
#include <Text.hpp>

struct StaticStyleEntry
{
    const char* name;
    const DefinitionStyle style;
    
    // For use by binary search (std::lower_bound())
    bool operator<(const StaticStyleEntry& other) const
    {
        return strcmp(name, other.name) < 0;
    }

    bool operator!=(const StaticStyleEntry& other) const
    {
        return strcmp(name, other.name) != 0;
    }
    
};

#define COLOR_NOT_DEF_INDEX -1
#define COLOR_DEF_INDEX 0
#define COLOR_NOT_DEF_COLOR 0

StaticAssert<COLOR_NOT_DEF_INDEX != COLOR_DEF_INDEX>;

#define COLOR_NOT_DEF  {COLOR_NOT_DEF_INDEX, COLOR_NOT_DEF_COLOR, COLOR_NOT_DEF_COLOR, COLOR_NOT_DEF_COLOR}
#define rgb(r,g,b)  {COLOR_DEF_INDEX,r,g,b}
#define WHITE    rgb(255,255,255)
#define BLACK    rgb(0,0,0)
#define GREEN    rgb(0,196,0)
#define BLUE     rgb(0,0,255)
#define RED      rgb(255,0,0)
#define YELLOW   rgb(255,255,0)
#define GRAY     rgb(127,127,127)

#define COLOR_UI_MENU_SELECTED_FILL   rgb(51,0,153)
#define COLOR_UI_FORM_FRAME           rgb(51,0,153)

#define FONT_NOT_DEF  FontID(-1)
#define NOT_DEF         DefinitionStyle::notDefined
#define YES            DefinitionStyle::yes
#define NO           DefinitionStyle::no
#define UNDERLINE_NOT_DEF UnderlineModeTag(-1)

#define COLOR(name, color) \
    {(name), {color, COLOR_NOT_DEF, FONT_NOT_DEF, NOT_DEF, NOT_DEF, NOT_DEF, NOT_DEF, NOT_DEF, NOT_DEF, UNDERLINE_NOT_DEF}}
#define COLOR_BOLD(name, color) \
    {(name), {color, COLOR_NOT_DEF, FONT_NOT_DEF, YES, NOT_DEF, NOT_DEF, NOT_DEF, NOT_DEF, NOT_DEF, UNDERLINE_NOT_DEF}}
#define COLOR_AND_FONT(name, color, font) \
    {(name), {color, COLOR_NOT_DEF, font, NOT_DEF, NOT_DEF, NOT_DEF, NOT_DEF, NOT_DEF, NOT_DEF, UNDERLINE_NOT_DEF}}
#define COLOR_AND_FONT_BOLD(name, color, font) \
    {(name), {color, COLOR_NOT_DEF, font, YES, NOT_DEF, NOT_DEF, NOT_DEF, NOT_DEF, NOT_DEF, UNDERLINE_NOT_DEF}}

// keep this array sorted!
static const StaticStyleEntry staticStyleTable[] =
{
    //do not touch .xxx styles (keep them 

/*
    {styleNameDefault, {BLACK, WHITE, stdFont, NO, NO, NO, NO, NO, NO, noUnderline}},
    {styleNameHyperlink, {COLOR_UI_FORM_FRAME, COLOR_NOT_DEF, FONT_NOT_DEF, NOT_DEF, NOT_DEF, NOT_DEF, NOT_DEF, NOT_DEF, NOT_DEF, grayUnderline}},

    COLOR(styleNameBlack,BLACK),
    COLOR(styleNameBlue,BLUE),
    COLOR_BOLD(styleNameBold,COLOR_NOT_DEF),
    COLOR_BOLD(styleNameBoldBlue,BLUE),
    COLOR_BOLD(styleNameBoldGreen,GREEN),
    COLOR_BOLD(styleNameBoldRed,RED),
    COLOR(styleNameGray,GRAY),
    COLOR(styleNameGreen,GREEN),
    COLOR_BOLD(styleNameHeader, COLOR_UI_FORM_FRAME),
    COLOR_AND_FONT(styleNameLarge, COLOR_NOT_DEF, largeFont),
    COLOR_AND_FONT(styleNameLargeBlue, BLUE, largeFont),
    COLOR_AND_FONT_BOLD(styleNamePageTitle, COLOR_UI_MENU_SELECTED_FILL, largeFont),
    COLOR(styleNameRed,RED),
    COLOR(styleNameSmallHeader, COLOR_UI_FORM_FRAME),
    COLOR_BOLD(styleNameStockPriceDown, RED),
    COLOR_BOLD(styleNameStockPriceUp, GREEN),
    COLOR(styleNameYellow,YELLOW)
 */
    //TODO: add more and more:)
};


uint_t getStaticStyleCount()
{
    return ARRAY_SIZE(staticStyleTable);
}

const char* getStaticStyleName(uint_t index)
{
    assert(index < ARRAY_SIZE(staticStyleTable));
    return staticStyleTable[index].name;
}

const DefinitionStyle* getStaticStyle(uint_t index)
{
    assert(index < ARRAY_SIZE(staticStyleTable));
    return &staticStyleTable[index].style;
}

const DefinitionStyle* getStaticStyle(const char* name, uint_t length)
{
    if ((uint_t)(-1) == length)
        length = strlen(name);

    if (NULL == name || 0 == length)
        return NULL;
    char* nameBuf = (char*)malloc(length + 1);
    if (NULL == nameBuf)
        return NULL;

	memcpy(nameBuf, name, length);
	nameBuf[length] = '\0';
    
    StaticStyleEntry entry = COLOR(nameBuf, COLOR_NOT_DEF);
    const StaticStyleEntry* end = staticStyleTable + ARRAY_SIZE(staticStyleTable);
    const StaticStyleEntry* res = std::lower_bound(staticStyleTable, end, entry);
    
    // return null if res != entry
    if (end != res)
        if (entry != *res)
        {
            free(nameBuf);
            return NULL;
        }
            
    free(nameBuf);

    if (end == res)
        return NULL;
    return &res->style; 
}

//always return style!
DefinitionStyle* parseStyle(const char* style, ulong_t length)
{
    DefinitionStyle* s = new_nt DefinitionStyle();
    s->reset();


    RGBColorType rgb;
    rgb.r = 128;
    rgb.b = 255;
    rgb.g = 0;
    rgb.index = 0;
    //TODO: this is only test...
    s->foregroundColor = rgb;

    return s;
}

void DefinitionStyle::reset()
{
    foregroundColor = (RGBColorType) COLOR_NOT_DEF;
    backgroundColor = (RGBColorType) COLOR_NOT_DEF;
    fontId = FONT_NOT_DEF;
    bold = NOT_DEF;
    italic = NOT_DEF;
    superscript = NOT_DEF;
    subscript = NOT_DEF;
    small = NOT_DEF;
    strike = NOT_DEF;
    underline = UNDERLINE_NOT_DEF;
}

inline static bool isColorDefined(const RGBColorType& col)
{
    return ((UInt8)COLOR_NOT_DEF_INDEX) != col.index;
}

DefinitionStyle& DefinitionStyle::operator|=(const DefinitionStyle& other)
{
    if (&other == NULL)
        return *this;
    if (NOT_DEF != other.bold)
        bold = other.bold;
    if (NOT_DEF != other.italic)
        italic = other.italic;
    if (NOT_DEF != other.small)
        small = other.small;
    if (NOT_DEF != other.strike)
        strike = other.strike;
    if (NOT_DEF != other.subscript)
        subscript = other.subscript;
    if (NOT_DEF != other.superscript)
        superscript = other.superscript;
    if (UNDERLINE_NOT_DEF != other.underline)
        underline = other.underline;

    if (FONT_NOT_DEF != other.fontId)
        fontId = other.fontId;
    if (isColorDefined(other.foregroundColor))
        foregroundColor = other.foregroundColor;
    if (isColorDefined(other.backgroundColor))
        backgroundColor = other.backgroundColor;
    return *this;
}


#ifdef DEBUG
void test_StaticStyleTable()
{
    // Validate that fields_ are sorted.
    for (uint_t i = 0; i < ARRAY_SIZE(staticStyleTable); ++i)
    {
        if (i > 0 && staticStyleTable[i] < staticStyleTable[i-1])
        {
            const char_t* prevName = staticStyleTable[i-1].name;
            const char_t* nextName = staticStyleTable[i].name;
            assert(false);
        }
    }
    
    //test |= operator on NULL style
    DefinitionStyle* ptr = NULL;
    DefinitionStyle s = *getStaticStyle(styleIndexDefault);
    s |= *ptr;
}
#endif
