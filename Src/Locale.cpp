
#include <Locale.hpp>


using namespace ArsLexis;

//! @todo as number of languages grows, switch to sorted array and binary search
const char_t* ArsLexis::GetLangNameByLangCode(const String& langCode)
{
    if (langCode.empty() || _T("en") == langCode)
    {
        return _T("English");
    }
    else if (_T("fr") == langCode)
    {
        return _T("French");
    }
    else if (_T("de") == langCode)        
    {
        return _T("German");
    }
    else
    {
        assert(false);
        return NULL;
    }
}
