#include <DefinitionStyle.hpp>
#include <Text.hpp>

long StyleParseFontWeight(const char* val, ulong_t len)
{
    long res = -1;
    status_t err = numericValue(val, val + len, res);
    if (errNone == res)
        return res;
    
    if (StrEquals(val, len, styleAttrValueFontWeightNormal))
        res = styleAttrValueFontWeightNormalNum;
    else if (StrEquals(val, len, styleAttrValueFontWeightLighter))
        res = 100;
    else if (StrEquals(val, len, styleAttrValueFontWeightBold))
        res = styleAttrValueFontWeightBoldNum;
    else if (StrEquals(val, len, styleAttrValueFontWeightBolder))
        res = 900;
    return res; 
}

long StyleParseFontSize(const char* value, ulong_t length)
{
    // TODO: implement StyleParseFontSize();
    return -1;
}

bool StyleParseColor(const char* value, ulong_t length, unsigned char& r, unsigned char& g, unsigned char& b)
{
    // TODO: implement StyleParseColor();
    return false;
}
