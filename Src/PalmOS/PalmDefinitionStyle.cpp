#include <DefinitionStyle.hpp>
#include <Text.hpp>
#include <Logging.hpp>
#include <Graphics.hpp>

static bool StyleParseColor(const char* val, ulong_t valLen, RGBColorType& color)
{
    if (!StyleParseColor(val, valLen, color.r, color.g, color.b))
        return false;
    color.index = COLOR_DEF_INDEX;
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

    // PalmOS doesn't support different font faces
    if (StrEquals(attr, attrLen, styleAttrNameFontFamily))
        return true;
        
    // PalmOS doesn't support font styles
    if (StrEquals(attr, attrLen, styleAttrNameFontStyle))
        return true;
        
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
        if (l < (styleAttrValueFontWeightNormalNum + styleAttrValueFontWeightBoldNum) / 2)
            style.bold = style.no;
        else
            style.bold = style.yes;
        return true;
    }
    
    if (StrEquals(attr, attrLen, styleAttrNameFontSize))
    {
        l = StyleParseFontSize(val, valLen);
        if (-1 == l)
            return false;
        if (l < (styleAttrValueFontSizeMediumNum + styleAttrValueFontSizeLargeNum) / 2)
            style.fontId = stdFont;
        else
            style.fontId = largeFont;
        return true;
    }
    
    if (StrEquals(attr, attrLen, styleAttrNameTextDecoration))
    {
        if (StrEquals(val, valLen, styleAttrValueTextDecorationNone))
        {
            style.strike = style.no;
            style.underline = noUnderline;
        }
        else if (StrEquals(val, valLen, styleAttrValueTextDecorationUnderline))
            style.underline = solidUnderline;
        else if (StrEquals(val, valLen, styleAttrValueTextDecorationXUnderlineDotted))
            style.underline = grayUnderline;
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
                Log(eLogInfo, "StyleParse(): failed to parse attribute: ", false);
                Log(eLogInfo, name, nameLen, false);
                Log(eLogInfo, "; value: ", false);
                Log(eLogInfo, value, valueLen, true);
                return false;
            }
        }
        curLen -= nextParam+1;
        start += nextParam+1;
    }
    return true;
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


const DefinitionStyle* StyleGetStaticStyleHelper(const StaticStyleEntry* array, uint_t arraySize, const char* name, uint_t length)
{
    bool copy = true;
    if (uint_t(-1) == length)
    {
        length = strlen(name);
        copy = false;
    }

    if (NULL == name || 0 == length)
        return NULL;
        
    char* nameBuf = (char*)name;
    
    if (copy)
    {
        nameBuf = StringCopy2N(name, length);
        if (NULL == nameBuf)
            return NULL;
    }
        
    StaticStyleEntry entry = COLOR(nameBuf, COLOR_NOT_DEF);
    const StaticStyleEntry* end = array + arraySize;
    const StaticStyleEntry* res = std::lower_bound(array, end, entry);
    
    // return null if res != entry
    if (end != res)
        if (entry != *res)
        {
            if (copy)
                free(nameBuf);
            return NULL;
        }
    
    if (copy)        
        free(nameBuf);

    if (end == res)
        return NULL;
    return &res->style; 
}



#ifndef NDEBUG

void test_StyleParse()
{
    const char* txt = 
" font-family: serif;\nfont-style:\n italic;\n\n \n \nfont-variant: small-caps\n;\n font-weight\n :\n bold\n ;font-size:large;color:rgb(\n90%,196, 0%\n);background-color:#fff;\n\n text-decoration: underline\n;vertical-align:baseline;";
    DefinitionStyle* style = StyleParse(txt, strlen(txt));
    assert(NULL != style);
    assert(largeFont == style->fontId);
    assert(style->yes == style->small);
    assert(style->yes == style->bold);
    assert(-1 != style->foregroundColor.index);
    assert(-1 != style->backgroundColor.index);
    assert(solidUnderline == style->underline);
    assert(style->no == style->subscript);
    assert(style->no == style->superscript);
    delete style;
}

#endif
