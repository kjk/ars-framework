#include <Debug.hpp>
#include <LangNames.hpp>
#include <Text.hpp>

using namespace ArsLexis;

struct LangCodeEntry 
{
    char_t code[3];
    const char_t* name;
    
    // For use by binary search (std::lower_bound())
    bool operator<(const LangCodeEntry& other) const
    {
        return tstrcmp(code, other.code) < 0;
    }
    
};

// This is taken from ISO 639 standard (http://www.w3.org/WAI/ER/IG/ert/iso639.htm)
static const LangCodeEntry langCodes[] =
{
    {"aa", "Afar"},
    {"ab", "Abkhazian"},
    {"af", "Afrikaans"},
    {"am", "Amharic"},
    {"ar", "Arabic"},
    {"as", "Assamese"},
    {"ay", "Aymara"},
    {"az", "Azerbaijani"},
    {"ba", "Bashkir"},
    {"be", "Byelorussian"},
    {"bg", "Bulgarian"},
    {"bh", "Bihari"},
    {"bi", "Bislama"},
    {"bn", "Bengali (Bangla)"},
    {"bo", "Tibetan"},
    {"br", "Breton"},
    {"ca", "Catalan"},
    {"co", "Corsican"},
    {"cs", "Czech"},
    {"cy", "Welsh"},
    {"da", "Danish"},
    {"de", "German"},
    {"dz", "Bhutani"},
    {"el", "Greek"},
    {"en", "English"},
    {"eo", "Esperanto"},
    {"es", "Spanish"},
    {"et", "Estonian"},
    {"eu", "Basque"},
    {"fa", "Persian"},
    {"fi", "Finnish"},
    {"fj", "Fiji"},
    {"fo", "Faeroese"},
    {"fr", "French"},
    {"fy", "Frisian"},
    {"ga", "Irish"},
    {"gd", "Gaelic"},
    {"gl", "Galician"},
    {"gn", "Guarani"},
    {"gu", "Gujarati"},
    {"ha", "Hausa"},
    {"hi", "Hindi"},
    {"hr", "Croatian"},
    {"hu", "Hungarian"},
    {"hy", "Armenian"},
    {"ia", "Interlingua"},
    {"ie", "Interlingue"},
    {"ik", "Inupiak"},
    {"in", "Indonesian"},
    {"is", "Icelandic"},
    {"it", "Italian"},
    {"iw", "Hebrew"},
    {"ja", "Japanese"},
    {"ji", "Yiddish"},
    {"jw", "Javanese"},
    {"ka", "Georgian"},
    {"kk", "Kazakh"},
    {"kl", "Greenlandic"},
    {"km", "Cambodian"},
    {"kn", "Kannada"},
    {"ko", "Korean"},
    {"ks", "Kashmiri"},
    {"ku", "Kurdish"},
    {"ky", "Kirghiz"},
    {"la", "Latin"},
    {"ln", "Lingala"},
    {"lo", "Laothian"},
    {"lt", "Lithuanian"},
    {"lv", "Latvian (Lettish)"},
    {"mg", "Malagasy"},
    {"mi", "Maori"},
    {"mk", "Macedonian"},
    {"ml", "Malayalam"},
    {"mn", "Mongolian"},
    {"mo", "Moldavian"},
    {"mr", "Marathi"},
    {"ms", "Malay"},
    {"mt", "Maltese"},
    {"my", "Burmese"},
    {"na", "Nauru"},
    {"ne", "Nepali"},
    {"nl", "Dutch"},
    {"no", "Norwegian"},
    {"oc", "Occitan"},
    {"om", "Oromo (Afan)"},
    {"or", "Oriya"},
    {"pa", "Punjabi"},
    {"pl", "Polish"},
    {"ps", "Pashto"},
    {"pt", "Portuguese"},
    {"qu", "Quechua"},
    {"rm", "Rhaeto-Romance"},
    {"rn", "Kirundi"},
    {"ro", "Romanian"},
    {"ru", "Russian"},
    {"rw", "Kinyarwanda"},
    {"sa", "Sanskrit"},
    {"sd", "Sindhi"},
    {"sg", "Sangro"},
    {"sh", "Serbo-Croatian"},
    {"si", "Singhalese"},
    {"sk", "Slovak"},
    {"sl", "Slovenian"},
    {"sm", "Samoan"},
    {"sn", "Shona"},
    {"so", "Somali"},
    {"sq", "Albanian"},
    {"sr", "Serbian"},
    {"ss", "Siswati"},
    {"st", "Sesotho"},
    {"su", "Sudanese"},
    {"sv", "Swedish"},
    {"sw", "Swahili"},
    {"ta", "Tamil"},
    {"te", "Tegulu"},
    {"tg", "Tajik"},
    {"th", "Thai"},
    {"ti", "Tigrinya"},
    {"tk", "Turkmen"},
    {"tl", "Tagalog"},
    {"tn", "Setswana"},
    {"to", "Tonga"},
    {"tr", "Turkish"},
    {"ts", "Tsonga"},
    {"tt", "Tatar"},
    {"tw", "Twi"},
    {"uk", "Ukrainian"},
    {"ur", "Urdu"},
    {"uz", "Uzbek"},
    {"vi", "Vietnamese"},
    {"vo", "Volapuk"},
    {"wo", "Wolof"},
    {"xh", "Xhosa"},
    {"yo", "Yoruba"},
    {"zh", "Chinese"},
    {"zu", "Zulu"}
};


const char_t* GetLangNameByLangCode(const char_t* code)
{
    if (NULL == code)
        return NULL;
    if (_T('\0') == *code)
        return NULL;
    
    LangCodeEntry entry = {_T("\0"), NULL};
    entry.code[0] = toLower(*code++);
    entry.code[1] = toLower(*code);
    entry.code[2] = _T('\0');
    
    const LangCodeEntry* end = langCodes + ARRAY_SIZE(langCodes);
    const LangCodeEntry* res = std::lower_bound(langCodes, end, entry);
    if (end == res)
        return NULL;
    
    return res->name; 
}

// Note: it has more languages than we support on the server currently.
// This is for backwards compatibility. We want to make sure that current
// client will also handle future languages when we add them on the server.
/*
const char_t* GetLangNameByLangCode(const String& langCode)
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
*/
