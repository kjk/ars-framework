
#include <Locale.hpp>


using namespace ArsLexis;

// Note: it has more languages than we support on the server currently.
// This is for backwards compatibility. We want to make sure that current
// client will also handle future languages when we add them on the server.
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
    else if (_T("es") == langCode)
    {
        return _T("Spanish");
    }
    else if (_T("it") == langCode)
    {
        return _T("Italian");
    }
    else if (_T("ja") == langCode)
    {
        return _T("Japanese");
    }
    else if (_T("nl") == langCode)
    {
        return _T("Dutch");
    }
    else if (_T("pl") == langCode)
    {
        return _T("Polish");
    }
    else if (_T("pt") == langCode)
    {
        return _T("Portuguese");
    }
    else if (_T("ru") == langCode)
    {
        return _T("Russian");
    }
    else if (_T("sv") == langCode)
    {
        return _T("Swedish");
    }
    else if (_T("zh") == langCode)
    {
        return _T("Chinese");
    }
    else if (_T("da") == langCode)
    {
        return _T("Danish");
    }
    else if (_T("eo") == langCode)
    {
        return _T("Esperanto");
    }
    else if (_T("he") == langCode)
    {
        return _T("Hebrew");
    }
    else if (_T("hu") == langCode)
    {
        return _T("Hungarian");
    }
    else if (_T("no") == langCode)
    {
        return _T("Norwegian");
    }
    else
    {
        assert(false);
        return NULL;
    }
}
