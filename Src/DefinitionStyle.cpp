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

long StyleParseFontSize(const char* val, ulong_t len)
{
    long res = -1;
    long pos = StrFind(val, len, "pt", -1);
    if (-1 != pos && long(len - 2) == pos)
    {
        status_t err = numericValue(val, val + pos, res);
        if (errNone == err && res > 0)
            return res;
    }

    if (StrEquals(val, len, styleAttrValueFontSizeXXSmall))
        res = styleAttrValueFontSizeMediumNum - 4;
    else if (StrEquals(val, len, styleAttrValueFontSizeXSmall))
        res = styleAttrValueFontSizeMediumNum - 3;
    else if (StrEquals(val, len, styleAttrValueFontSizeSmall))
        res = styleAttrValueFontSizeMediumNum - 2;
    else if (StrEquals(val, len, styleAttrValueFontSizeMedium))
        res = styleAttrValueFontSizeMediumNum;
    else if (StrEquals(val, len, styleAttrValueFontSizeLarge))
        res = styleAttrValueFontSizeLargeNum;
    else if (StrEquals(val, len, styleAttrValueFontSizeXLarge))
        res = styleAttrValueFontSizeLargeNum + 4;
    else if (StrEquals(val, len, styleAttrValueFontSizeXXLarge))
        res = styleAttrValueFontSizeLargeNum + 8;
        
    return res;
}

static bool StyleParseColorHashed(const char* val, ulong_t len, unsigned char& r, unsigned char& g, unsigned char& b)
{	
    if (6 != len && 3 != len)
        return false;
    
	typedef unsigned char uchar;
    long step = len / 3;
    long c;
    status_t err;
    for (long i = 0; i < 3; ++i)
    {
        err = numericValue(val, val + step, c, 16);
        if (errNone != err)
            return false;
        
        val += step;
        
        if (c < 0)
            return false;
            
        if (1 == step)
            c *= 17;
        
        switch (i)
        {
            case 0:
                r = uchar(c);
                break;
            case 1:
                g = uchar(c);
                break;
            case 2:
                b = uchar(c);
                break;
        }
    }
    return true;
}

static bool StyleParseColorComponent(const char* val, ulong_t len, unsigned char& c)
{
    strip(val, len);
    if (0 == len)
        return false;
    
    bool percent = false;
    if ('%' == val[len - 1])
    {
        percent = true;
        --len;
    }
    
    long res;
    status_t err = numericValue(val, val + len, res);
    if (errNone != err)
        return false;
    
    if (res < 0)
        return false;
        
    if (percent)
        res = (res * 255) / 100;

    res = std::min<long>(255, res);
    
    c = (unsigned char)res;
    return true;
}

static bool StyleParseColorTriplet(const char* val, ulong_t len, unsigned char& r, unsigned char& g, unsigned char& b)
{
    long pos = StrFind(val, len, ',');
    if (-1 == pos)
        return false;
    
    if (!StyleParseColorComponent(val, pos, r))
        return false;
    
    val += (pos + 1);
    len -= (pos + 1);
        
    pos = StrFind(val, len, ',');
    if (-1 == pos)
        return false;
        
    if (!StyleParseColorComponent(val, pos, g))
        return false;

    val += (pos + 1);
    len -= (pos + 1);

    if (!StyleParseColorComponent(val, len, b))
        return false;
        
    return true;
}

bool StyleParseColor(const char* val, ulong_t len, unsigned char& r, unsigned char& g, unsigned char& b)
{
    if (StrStartsWith(val, len, "#", 1))
        return StyleParseColorHashed(++val, --len, r, g, b);
    else if (StrStartsWith(val, len, "rgb(", 4) && ')' == val[len - 1])
        return StyleParseColorTriplet(val + 4, len - 5, r, g, b);
    return false;
}

