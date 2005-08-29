#include <DefinitionStyle.hpp>
#include <Graphics.hpp>
#include <Debug.hpp>
#include <Text.hpp>

#include <wingdi.h>
#include <WindowsCE/shguim.h>

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

    
	LOGFONT font, system;
	
	
    HGDIOBJ f = GetStockObject(SYSTEM_FONT);
    GetObject(f, sizeof(system), &system);
    DeleteObject(f); 
       
	ZeroMemory(&font, sizeof(font));

	long height;
	if (fontSizeNotDefined == fontSize)
		height = fontSizeNormal;
	else
		height = fontSize;

	if (yes == superscript || yes == subscript || yes == small)
	{
		height *= 2;
		height /= 3;
	}
    
	LONG  dwFontSize = 800;
	SHGetUIMetrics(SHUIM_FONTSIZE_POINT, &dwFontSize, sizeof(dwFontSize),  NULL);
	HDC  dc = GetWindowDC(NULL);
	int dpi = GetDeviceCaps(dc, LOGPIXELSY);
	ReleaseDC(NULL, dc);
	font.lfHeight = - ((height * dwFontSize * dpi + fontSizeNormal * 3600) / (fontSizeNormal * 7200));


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

//#ifdef SPI_GETFONTSMOOTHING
//
//	BOOL en = FALSE;
//	BOOL res = SystemParametersInfo(SPI_GETFONTSMOOTHING, 0, &en, FALSE);
//	if (res && en)
//	    font.lfQuality = CLEARTYPE_COMPAT_QUALITY;
//    else    
//#endif
    	font.lfQuality = DEFAULT_QUALITY;

	if (fontFamilyNotDefined == fontFamily)
	{
		font.lfPitchAndFamily = fontFamilyNormal;
        memcpy(font.lfFaceName, system.lfFaceName, sizeof(font.lfFaceName));
    }
	else
		font.lfPitchAndFamily = fontFamily;

	cachedFont_.createIndirect(font);
	return cachedFont_;
}

void DefinitionStyle::invalidateCachedFont() const
{
	if (cachedFont_.valid())
		cachedFont_ = WinFont();
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
		else if (StrEquals(val, valLen, styleAttrValueFontStyleItalic))
			style.italic = style.yes;
		else if (StrEquals(val, valLen, styleAttrValueFontStyleObligue))
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

bool StyleParse(DefinitionStyle& out, const char* style, ulong_t length)
{
	out.reset();

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
            if (!StyleParseAttribute(name, nameLen, value, valueLen, out))
            {
/*                
				Log(eLogInfo, "StyleParse(): failed to parse attribute: ", false);
                Log(eLogInfo, name, nameLen, false);
                Log(eLogInfo, "; value: ", false);
                Log(eLogInfo, value, valueLen, true);
 */
				return false;
            }
        }
        curLen -= nextParam+1;
        start += nextParam+1;
    }
	return true;
}


