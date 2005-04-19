#include <DefinitionStyle.hpp>
#include <Graphics.hpp>
#include <Debug.hpp>
#include <Text.hpp>

#include <wingdi.h>

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

	HDC dc = GetDC(NULL);
	font.lfHeight = - (height * GetDeviceCaps(dc, LOGPIXELSY)) / 72;
	ReleaseDC(NULL, dc);

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

	font.lfCharSet = ANSI_CHARSET;
	font.lfOutPrecision = OUT_DEFAULT_PRECIS;

	font.lfClipPrecision = CLIP_DEFAULT_PRECIS;

#ifdef _WIN32_WCE
#if _WIN32_WCE > 400
	font.lfQuality = CLEARTYPE_QUALITY;
#else
	font.lfQuality = DEFAULT_QUALITY;
//	font.lfQuality = ANTIALIASED_QUALITY;
#endif
#endif

	// font.lfQuality = CLEARTYPE_COMPAT_QUALITY;

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
	{styleNameHyperlink, "text-decoration: underline; color: rgb(0, 0, 100%);"},
	{styleNameBlack, "color: rgb(0, 0, 0);"},
	{styleNameBlue, "color: rgb(0, 0, 100%);"},
	{styleNameBold, "font-weight: bold;"},
	{styleNameBoldBlue, "font-weight: bold; color: rgb(0, 0, 100%);"},
	{styleNameBoldGreen, "font-weight: bold; color: rgb(0, 100%, 0);"},
	{styleNameBoldRed, "font-weight: bold; color: rgb(100%, 0, 0);"},
	{styleNameGray, "color: rgb(67%, 67%, 67%);"},
	{styleNameGreen, "color: rgb(0, 100%, 0);"},
	{styleNameHeader, "font-size: large; font-weight: bold;"},
	{styleNameLarge, "font-size: large;"},
	{styleNameLargeBlue, "font-size: large; color: rgb(0, 0, 100%);"},
	{styleNamePageTitle, "font-size: x-large; font-weight: bold;"},
	{styleNameRed, "color: rgb(100%, 0, 0);"},
	{styleNameSmallHeader, "font-weight: bold;"},
	{styleNameStockPriceDown, "font-weight: bold; color: rgb(100%, 0, 0);"},
	{styleNameStockPriceUp, "font-weight: bold; color: rgb(0, 100%, 0);"},
	{styleNameYellow, "color: rgb(100%, 100%, 0);"},
};

static DefinitionStyle* staticStyles[ARRAY_SIZE(staticStyleDescriptors)] = {NULL};

/*

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
	assert(NULL != staticStyles[index]);
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

static bool StyleParseColor(const char* val, ulong_t valLen, COLORREF& color)
{
	unsigned char r, g, b;
    if (!StyleParseColor(val, valLen, r, g, b))
        return false;
    color = RGB(r, g, b);
    return true;
}

static bool StyleParseAttribute(const char* attr, ulong_t attrLen, const char* val, ulong_t valLen, DefinitionStyle& style)
{
    long l;

    if (0 == attrLen)
        return false;
        
    if (StrEquals(attr, attrLen, styleAttrNameColor))
        return StyleParseColor(val, valLen, style.foregroundColor);
    
    if (StrEquals(attr, attrLen, styleAttrNameBackgroundColor))
        return StyleParseColor(val, valLen, style.backgroundColor);

    if (StrEquals(attr, attrLen, styleAttrNameFontFamily))
	{
		if (StrEquals(val, valLen, styleAttrValueFontFamilySerif))
			style.fontFamily = style.fontFamilySerif;
		else if (StrEquals(val, valLen, styleAttrValueFontFamilySansSerif))
			style.fontFamily = style.fontFamilySansSerif;
		else if (StrEquals(val, valLen, styleAttrValueFontFamilyCursive))
			style.fontFamily = style.fontFamilyCursive;
		else if (StrEquals(val, valLen, styleAttrValueFontFamilyFantasy))
			style.fontFamily = style.fontFamilyFantasy;
		else if (StrEquals(val, valLen, styleAttrValueFontFamilyMonospace))
			style.fontFamily = style.fontFamilyMonospace;
		else
			return false;
		// TODO: add support for custom font families
        return true;
	}
        
    if (StrEquals(attr, attrLen, styleAttrNameFontStyle))
	{
		if (StrEquals(val, valLen, styleAttrValueFontStyleNormal))
			style.italic = style.no;
		if (StrEquals(val, valLen, styleAttrValueFontStyleItalic))
			style.italic = style.yes;
		if (StrEquals(val, valLen, styleAttrValueFontStyleObligue))
			style.italic = style.yes;
		else
			return false;
        return true;
	}
        
    if (StrEquals(attr, attrLen, styleAttrNameFontVariant))
    {
        if (StrEquals(val, valLen, styleAttrValueFontVariantSmallCaps))
            style.small = style.yes;
        else if (StrEquals(val, valLen, styleAttrValueFontVariantNormal))
            style.small = style.no;
        else
            return false;
        return true;
    }
    
    if (StrEquals(attr, attrLen, styleAttrNameFontWeight))
    {
        l = StyleParseFontWeight(val, valLen);
        if (-1 == l)
            return false;
		style.fontWeight = l;
		return true;
    }
    
    if (StrEquals(attr, attrLen, styleAttrNameFontSize))
    {
        l = StyleParseFontSize(val, valLen);
        if (-1 == l)
            return false;
		style.fontSize = l;
		return true;
    }
    
    if (StrEquals(attr, attrLen, styleAttrNameTextDecoration))
    {
        if (StrEquals(val, valLen, styleAttrValueTextDecorationNone))
        {
            style.strike = style.no;
            style.underline = style.no;
        }
        else if (StrEquals(val, valLen, styleAttrValueTextDecorationUnderline))
            style.underline = style.yes;
        else if (StrEquals(val, valLen, styleAttrValueTextDecorationXUnderlineDotted))
            style.underline = style.yes;
        else if (StrEquals(val, valLen, styleAttrValueTextDecorationLineThrough))
            style.strike = style.yes;
        else
            return false;
        return true;
    }
    
    if (StrEquals(attr, attrLen, styleAttrNameVerticalAlign))
    {
        if (StrEquals(val, valLen, styleAttrValueVerticalAlignBaseline))
        {
            style.subscript = style.no;
            style.superscript = style.no;
        }
        else if (StrEquals(val, valLen, styleAttrValueVerticalAlignSubscript))
            style.subscript = style.yes;
        else if (StrEquals(val, valLen, styleAttrValueVerticalAlignSuperscript))
            style.superscript = style.yes;
        else
            return false;
        return true;
    }
        
    return false;
}

DefinitionStyle* StyleParse(const char* style, ulong_t length)
{
    DefinitionStyle* s = new_nt DefinitionStyle();
    if (NULL == s)
        return s;
        
    long curLen = length;
    const char* start = style;
    while (curLen > 0)
    {
        long nextParam = StrFind(start, curLen, ';');
        if (-1 == nextParam)
            nextParam = curLen;
        long nameEnd = StrFind(start, nextParam, ':');
        if (-1 != nameEnd)
        {
            const char* name = start;
            ulong_t nameLen = nameEnd;
            const char* value = start + nameEnd+1;
            ulong_t valueLen = nextParam - nameEnd-1;
            strip(value, valueLen);
            strip(name, nameLen);
            if (!StyleParseAttribute(name, nameLen, value, valueLen, *s))
            {
/*                
				Log(eLogInfo, "StyleParse(): failed to parse attribute: ", false);
                Log(eLogInfo, name, nameLen, false);
                Log(eLogInfo, "; value: ", false);
                Log(eLogInfo, value, valueLen, true);
 */
            }
        }
        curLen -= nextParam+1;
        start += nextParam+1;
    }
    return s;
}



void PrepareStaticStyles()
{
	assert(NULL == staticStyles[0]);
	for (uint_t i = 0; i < ARRAY_SIZE(staticStyleDescriptors); ++i)
	{
		const char* def = staticStyleDescriptors[i].definition;
		staticStyles[i] = StyleParse(def, strlen(def));
		// pre-create default fonts so that they are referenced at least once when used for the 1st time
		staticStyles[i]->font();
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
