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

const COLORREF DefinitionStyle::colorNotDefined = DWORD(-1); // COLORREF is DWORD of this form: 0x00bbggrr

static void SyncTriState(DefinitionStyle::TriState& self, const DefinitionStyle::TriState& other, bool& useThis, bool& useOther)
{
	if (DefinitionStyle::notDefined != other && self != other)
	{
		useThis = false;
		self = other;
	}
	else if (DefinitionStyle::notDefined != self)
		useOther = false;
}

DefinitionStyle& DefinitionStyle::operator|=(const DefinitionStyle& other)
{
	// TODO: remove this check when styles are finished
	if (NULL == &other)
		return *this;

	bool useThisFont = true;
	bool useOtherFont = true;

	if (colorNotDefined != other.foregroundColor)
		foregroundColor = other.foregroundColor;

	if (colorNotDefined != other.backgroundColor)
		backgroundColor = other.backgroundColor;

	if (fontSizeNotDefined != other.fontSize && fontSize != other.fontSize)
	{
		useThisFont = false;
		fontSize = other.fontSize;
	}
	else if (fontSizeNotDefined != fontSize)
		useOtherFont = false;

	if (fontWeightNotDefined != other.fontWeight && fontWeight != other.fontWeight)
	{
		useThisFont = false;
		fontWeight = other.fontWeight;
	}
	else if (fontWeightNotDefined != fontWeight)
		useOtherFont = false;

	SyncTriState(italic, other.italic, useThisFont, useOtherFont);

	// TODO: handle super- and subscript differently (and small)
	SyncTriState(superscript, other.superscript, useThisFont, useOtherFont);
	SyncTriState(subscript, other.subscript, useThisFont, useOtherFont);
	SyncTriState(small, other.small, useThisFont, useOtherFont);

	SyncTriState(strike, other.strike, useThisFont, useOtherFont);
	SyncTriState(underline, other.underline, useThisFont, useOtherFont);

	if (fontFamilyNotDefined != other.fontFamily && fontFamily != other.fontFamily)
	{
		useThisFont = false;
		fontFamily = other.fontFamily;
	}
	else if (fontFamilyNotDefined != fontFamily)
		useOtherFont = false;

	if (useThisFont)
		return *this;

	if (useOtherFont)
	{
		cachedFont_ = other.cachedFont_;
		return *this;
	}

	cachedFont_ = WinFont();
	return *this;
}

void DefinitionStyle::reset()
{
	backgroundColor = foregroundColor = colorNotDefined;
	fontSize = fontSizeNotDefined;
	fontWeight = fontWeightNotDefined;
	italic = superscript = subscript = small = strike = underline = notDefined;
	fontFamily = fontFamilyNotDefined;
	cachedFont_ = WinFont();
}

DefinitionStyle::DefinitionStyle()
{
	reset();
}

DefinitionStyle::~DefinitionStyle()
{
}

const WinFont& DefinitionStyle::font() const
{
	if (cachedFont_.valid())
		return cachedFont_;

	LOGFONT font;
	ZeroMemory(&font, sizeof(font));

	long height;
	if (fontSizeNotDefined == fontSize)
		height = fontSizeNormal;
	else
		height	 = fontSize;

	if (yes == superscript || yes == subscript || yes == small)
		height /= 2;
	font.lfHeight = height;

	if (fontWeightNotDefined == fontWeight)
		font.lfWeight = fontWeightNormal;
	else
		font.lfWeight = fontWeight;

	if (yes == italic)
		font.lfItalic = TRUE;

	if (yes == underline)
		font.lfUnderline = TRUE;

	if (yes == strike)
		font.lfStrikeOut = TRUE;

	font.lfCharSet = DEFAULT_CHARSET;
	font.lfOutPrecision = OUT_DEFAULT_PRECIS;
	font.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	font.lfQuality = CLEARTYPE_QUALITY;

	if (fontFamilyNotDefined == fontFamily)
		font.lfPitchAndFamily = fontFamilyNormal;
	else
		font.lfPitchAndFamily = fontFamily;

	cachedFont_.createIndirect(font);
	return cachedFont_;
}

struct StaticStyleDescriptor {
	const char* name;
	const char* definition;

	bool operator < (const StaticStyleDescriptor& des) const {return strcmp(name, des.name) < 0;}
};

static const StaticStyleDescriptor staticStyleDescriptors[] = {
	{styleNameDefault, "color: rgb(0, 0, 0); background-color: rgb(100%, 255, 255);"},
	{styleNameHyperlink, "text-decoration: underline; color: rgb(0, 0, 100%);"}
};

static DefinitionStyle* staticStyles[ARRAY_SIZE(staticStyleDescriptors)] = {NULL};


// keep this array sorted!
/*
static const StaticStyleEntry staticStyleTable[] =
{
    //do not touch .xxx styles (keep them 

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
    //TODO: add more and more:)
};
 */


uint_t getStaticStyleCount()
{
    return ARRAY_SIZE(staticStyles);
}

const char* getStaticStyleName(uint_t index)
{
    assert(index < ARRAY_SIZE(staticStyleDescriptors));
    return staticStyleDescriptors[index].name;
}

const DefinitionStyle* getStaticStyle(uint_t index)
{
    assert(index < ARRAY_SIZE(staticStyleDescriptors));
    return staticStyles[index];
}

const DefinitionStyle* getStaticStyle(const char* name, uint_t length)
{
    if (uint_t(-1) == length)
        length = strlen(name);

    if (NULL == name || 0 == length)
        return NULL;

    char* nameBuf = (char*)malloc(length + 1);
    if (NULL == nameBuf)
        return NULL;

	memcpy(nameBuf, name, length);
	nameBuf[length] = '\0';

	StaticStyleDescriptor des = {nameBuf, NULL};
	const StaticStyleDescriptor* end = staticStyleDescriptors + ARRAY_SIZE(staticStyleDescriptors);
	const StaticStyleDescriptor* res = std::lower_bound(staticStyleDescriptors, end, des);
	if (end == res || 0 != strcmp(nameBuf, res->name))
	{
		free(nameBuf);
		return NULL;
	}
    
	uint_t index = (res - staticStyleDescriptors);
	free(nameBuf);
	return staticStyles[index];
}

DefinitionStyle* parseStyle(const char* style, ulong_t length)
{
    DefinitionStyle* s = new_nt DefinitionStyle();
	if (NULL == s)
		return NULL;

	// TODO: actually... parse the style ;-)
	
    return s;
}

void PrepareStaticStyles()
{
	assert(NULL == staticStyles[0]);
	for (uint_t i = 0; i < ARRAY_SIZE(staticStyleDescriptors); ++i)
	{
		const char* def = staticStyleDescriptors[i].definition;
		staticStyles[i] = parseStyle(def, strlen(def));
	}

#ifdef DEBUG
	test_StaticStyleTable();
#endif
}

void DisposeStaticStyles()
{
	for (uint_t i = 0; i < ARRAY_SIZE(staticStyleDescriptors); ++i)
	{
		delete staticStyles[i];
		staticStyles[i] = NULL;
	}
}

#ifdef DEBUG
void test_StaticStyleTable()
{
    // Validate that fields_ are sorted.
    for (uint_t i = 0; i < ARRAY_SIZE(staticStyleDescriptors); ++i)
    {
        if (i > 0 && staticStyleDescriptors[i] < staticStyleDescriptors[i-1])
        {
            const char* prevName = staticStyleDescriptors[i-1].name;
            const char* nextName = staticStyleDescriptors[i].name;
            assert(false);
        }
    }
    
    //test |= operator on NULL style
    DefinitionStyle* ptr = NULL;
    DefinitionStyle s = *getStaticStyle(styleIndexDefault);
    s |= *ptr;
}
#endif
