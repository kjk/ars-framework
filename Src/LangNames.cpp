#include <Debug.hpp>
#include <LangNames.hpp>
#include <Text.hpp>

struct LangCodeEntry 
{
    const char_t* code;
    const char_t* name;
    
    // For use by binary search (std::lower_bound())
    bool operator<(const LangCodeEntry& other) const
    {
        return tstrcmp(code, other.code) < 0;
    }
    
};

#ifdef _WIN32_WCE
#define ALL_LANGS 1
#else
#define ALL_LANGS 0
#endif

// taken from: http://meta.wikimedia.org/wiki/Complete_list_of_language_Wikipedias_available
static const LangCodeEntry langCodes[] =
{
#if ALL_LANGS
    {_T("aa"), _T("Afar")},
    {_T("ab"), _T("Abkhazian")},
    {_T("af"), _T("Afrikaans")},
    {_T("ak"), _T("Akan")},
    {_T("als"), _T("Alemannic")},
    {_T("am"), _T("Amharic")},
    {_T("an"), _T("Aragonese")},
    {_T("ang"), _T("Anglo-Saxon (Old English)")},
    {_T("ar"), _T("Arabic")},
    {_T("arc"), _T("Aramaic")},
    {_T("as"), _T("Assamese")},
    {_T("ast"), _T("Asturian")},
    {_T("av"), _T("Avar")},
    {_T("ay"), _T("Aymara")},
    {_T("az"), _T("Azeri")},
    {_T("ba"), _T("Bashkir")},
    {_T("be"), _T("Belarusian")},
#endif
    {_T("bg"), _T("Bulgarian")},
#if ALL_LANGS
    {_T("bh"), _T("Bhojpuri")},
    {_T("bi"), _T("Bislama")},
    {_T("bm"), _T("Bambara")},
    {_T("bn"), _T("Bengali")},
    {_T("bo"), _T("Tibetan")},
    {_T("br"), _T("Breton")},
    {_T("bs"), _T("Bosnian")},
#endif    
    {_T("ca"), _T("Catalan")},
#if ALL_LANGS    
    {_T("ce"), _T("Chechen")},
    {_T("ch"), _T("Chamorro")},
    {_T("cho"), _T("Choctaw")},
    {_T("chr"), _T("Cherokee")},
    {_T("chy"), _T("Cheyenne")},
    {_T("co"), _T("Corsican")},
    {_T("cr"), _T("Cree")},
    {_T("cs"), _T("Czech")},
    {_T("csb"), _T("Kashubian")},
    {_T("cv"), _T("Chuvash")},
    {_T("cy"), _T("Welsh")},
#endif
    {_T("da"), _T("Danish")},
    {_T("de"), _T("German")},
#if ALL_LANGS    
    {_T("dv"), _T("Dhivehi")},
    {_T("dz"), _T("Dzongkha")},
    {_T("ee"), _T("Ewe")},
    {_T("el"), _T("Greek")},
#endif
    {_T("en"), _T("English")},
    {_T("eo"), _T("Esperanto")},
    {_T("es"), _T("Spanish")},
#if ALL_LANGS    
    {_T("et"), _T("Estonian")},
    {_T("eu"), _T("Basque")},
    {_T("fa"), _T("Persian")},
    {_T("ff"), _T("Fulfulde")},
#endif    
    {_T("fi"), _T("Finnish")},
#if ALL_LANGS    
    {_T("fj"), _T("Fijian")},
    {_T("fo"), _T("Faeroese")},
#endif
    {_T("fr"), _T("French")},
#if ALL_LANGS    
    {_T("fur"), _T("Friulian")},
    {_T("fy"), _T("Frisian")},
    {_T("ga"), _T("Irish")},
    {_T("gd"), _T("Scottish Gaelic")},
    {_T("gl"), _T("Galician")},
    {_T("gn"), _T("Guarani")},
    {_T("got"), _T("Gothic")},
    {_T("gu"), _T("Gujarati")},
    {_T("gv"), _T("Manx")},
    {_T("ha"), _T("Hausa")},
    {_T("haw"), _T("Hawaiian")},
#endif
    {_T("he"), _T("Hebrew")},
#if ALL_LANGS    
    {_T("hi"), _T("Hindi")},
    {_T("ho"), _T("Hiri")},
    {_T("hr"), _T("Croatian")},
    {_T("ht"), _T("Haitian")},
    {_T("hu"), _T("Hungarian")},
    {_T("hy"), _T("Armenian")},
    {_T("hz"), _T("Herero")},
    {_T("ia"), _T("Interlingua")},
    {_T("id"), _T("Indonesian")},
    {_T("ie"), _T("Interlingue")},
    {_T("ig"), _T("Igbo")},
    {_T("ii"), _T("Yi")},
    {_T("ik"), _T("Inupiaq")},
    {_T("io"), _T("Ido")},
    {_T("is"), _T("Icelandic")},
#endif 
    {_T("it"), _T("Italian")},
#if ALL_LANGS    
    {_T("iu"), _T("Inuktitut")},
#endif    
    {_T("ja"), _T("Japanese")},
#if ALL_LANGS    
    {_T("jv"), _T("Javanese")},
    {_T("ka"), _T("Georgian")},
    {_T("kg"), _T("Kongo")},
    {_T("ki"), _T("Kikuyu")},
    {_T("kk"), _T("Kazakh")},
    {_T("kl"), _T("Greenlandic")},
    {_T("km"), _T("Khmer")},
    {_T("kn"), _T("Kannada")},
    {_T("ko"), _T("Korean")},
    {_T("kr"), _T("Kanuri")},
    {_T("ks"), _T("Kashmiri")},
    {_T("ku"), _T("Kurdish")},
    {_T("kv"), _T("Komi")},
    {_T("kw"), _T("Cornish")},
    {_T("ky"), _T("Kyrgyz")},
    {_T("la"), _T("Latin")},
    {_T("lb"), _T("Luxembourgish")},
    {_T("lg"), _T("Luganda")},
    {_T("li"), _T("Limburgish")},
    {_T("ln"), _T("Lingala")},
    {_T("lo"), _T("Lao")},
    {_T("lt"), _T("Lithuanian")},
    {_T("mg"), _T("Malagasy")},
    {_T("mh"), _T("Marshallese")},
    {_T("mi"), _T("Maori")},
    {_T("mk"), _T("Macedonian")},
    {_T("ml"), _T("Malayalam")},
    {_T("mn"), _T("Mongolian")},
    {_T("mo"), _T("Moldovan")},
    {_T("ms"), _T("Malay")},
    {_T("mt"), _T("Maltese")},
    {_T("mus"), _T("Muscogee")},
    {_T("my"), _T("Burmese")},
    {_T("na"), _T("Nauruan")},
    {_T("nah"), _T("Nahuatl")},
    {_T("nds"), _T("Low Saxon")},
    {_T("ne"), _T("Nepali")},
#endif
    {_T("nl"), _T("Dutch")},
    {_T("nn"), _T("Nynorsk")},
    {_T("no"), _T("Norwegian")},
#if ALL_LANGS    
    {_T("nv"), _T("Navajo")},
    {_T("ny"), _T("Chichewa")},
    {_T("oc"), _T("Occitan")},
    {_T("or"), _T("Oriya")},
    {_T("pa"), _T("Punjabi")},
#endif
    {_T("pl"), _T("Polish")},
#if ALL_LANGS    
    {_T("ps"), _T("Pushtu")},
#endif
    {_T("pt"), _T("Portuguese")},
#if ALL_LANGS    
    {_T("qu"), _T("Quechua")},
    {_T("rm"), _T("Romansh")},
    {_T("rn"), _T("Kirundi")},
#endif
    {_T("ro"), _T("Romanian")},
#if ALL_LANGS    
    {_T("roa-rup"), _T("Aromanian")},
#endif
    {_T("ru"), _T("Russian")},
#if ALL_LANGS    
    {_T("rw"), _T("Kinyarwanda")},
    {_T("sa"), _T("Sanskrit")},
    {_T("sc"), _T("Sardinian")},
    {_T("scn"), _T("Sicilian")},
    {_T("sd"), _T("Sindhi")},
    {_T("sg"), _T("Sango")},
    {_T("sh"), _T("Serbo-Croatian")},
    {_T("si"), _T("Sinhala")},
    {_T("simple"), _T("Simple English")},
    {_T("sk"), _T("Slovak")},
#endif    
    {_T("sl"), _T("Slovenian")},
#if ALL_LANGS    
    {_T("sm"), _T("Samoan")},
    {_T("so"), _T("Somali")},
    {_T("sr"), _T("Serbian")},
    {_T("ss"), _T("Swati")},
    {_T("st"), _T("Sesotho")},
    {_T("st"), _T("Sotho")},
    {_T("su"), _T("Sundanese")},
#endif
    {_T("sv"), _T("Swedish")},
#if ALL_LANGS    
    {_T("sw"), _T("Swahili")},
    {_T("ta"), _T("Tamil")},
    {_T("te"), _T("Telugu")},
    {_T("tg"), _T("Tajik")},
    {_T("th"), _T("Thai")},
    {_T("ti"), _T("Tigrignan")},
    {_T("tl"), _T("Tagalog")},
    {_T("tlh"), _T("Klingon")},
    {_T("tn"), _T("Setswana")},
    {_T("tpi"), _T("Tok Pisin")},
    {_T("tr"), _T("Turkish")},
    {_T("tt"), _T("Tatar")},
    {_T("tum"), _T("Tumbuka")},
    {_T("ty"), _T("Tahitian")},
    {_T("ug"), _T("Uyghur")},
#endif
    {_T("uk"), _T("Ukrainian")},
#if ALL_LANGS    
    {_T("ur"), _T("Urdu")},
    {_T("uz"), _T("Uzbek")},
    {_T("ve"), _T("Venda")},
    {_T("vi"), _T("Vietnamese")},
    {_T("vo"), _T("Volapuk")},
    {_T("wa"), _T("Walloon")},
    {_T("wo"), _T("Wolof")},
    {_T("yi"), _T("Yiddish")},
    {_T("yo"), _T("Yoruba")},
    {_T("za"), _T("Zhuang")},
#endif
    {_T("zh"), _T("Chinese")},
#if ALL_LANGS    
    {_T("zh-min-nan"), _T("Min Nan")},
    {_T("zu"), _T("Zulu")}
#endif
};

static const char_t* GetLangNameProper(const char_t* code)
{
    LangCodeEntry entry = {code, NULL};
    
    const LangCodeEntry* end = langCodes + ARRAY_SIZE(langCodes);
    const LangCodeEntry* res = std::lower_bound(langCodes, end, entry);
    if (end == res)
        return NULL;

	if (!StrEquals(code, res->code))
		return NULL;
    
    return res->name; 
}

const char_t* GetLangNameByLangCode(const char* code, long length)
{
    if (NULL == code) 
		return NULL;
        
    if (-1 == length) length = Len(code);
    if (0 == length || length > 10)
        return NULL;

    char_t codeBuf[11];
    long i;
    for (i = 0; i < length; ++i)
	{
		char_t chr = char_t((unsigned char)code[i]);
		assert(chr < 128); 
        codeBuf[i] = toLower(chr);
	}
    codeBuf[i] = _T('\0');
   
   return GetLangNameProper(codeBuf); 
}

#ifdef _WIN32_WCE
const wchar_t* GetLangNameByLangCode(const wchar_t* code, long length)
{
    if (NULL == code)
        return NULL;
        
    if (-1 == length) length = Len(code);
    if (0 == length || length > 10)
        return NULL;
    
    char_t codeBuf[11];
    long i;
    for (i = 0; i < length; ++i)
        codeBuf[i] = toLower(code[i]);
    codeBuf[i] = L'\0';
   
   return GetLangNameProper(codeBuf); 
}
#endif
