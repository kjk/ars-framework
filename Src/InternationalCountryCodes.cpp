#include <InternationalCountryCodes.hpp>
#include <Text.hpp>

#ifdef __MWERKS__
# pragma pcrelconstdata on
#endif

namespace ArsLexis{

    enum {
        countryAbbrevLength=3,
        countryNameLength=40, // 38+1 
    };
    
    typedef const char_t CountryAbbrevField_t[countryAbbrevLength];
    typedef const char_t CountryNameField_t[countryNameLength];
    
    struct CountryArrayEntry {
        const CountryAbbrevField_t abbrev;
        const CountryNameField_t name; 
    };
    
    static const CountryArrayEntry countries[]={
        {_T("AF"), _T("Afghanistan, Islamic State of")},
        {_T("AL"), _T("Albania")},
        {_T("DZ"), _T("Algeria")},
        {_T("AS"), _T("American Samoa")},
        {_T("AD"), _T("Andorra, Principality of")},
        {_T("AO"), _T("Angola")},
        {_T("AI"), _T("Anguilla")},
        {_T("AQ"), _T("Antarctica")},
        {_T("AG"), _T("Antigua and Barbuda")},
        {_T("AR"), _T("Argentina")},
        {_T("AM"), _T("Armenia")},
        {_T("AW"), _T("Aruba")},
        {_T("AC"), _T("Ascension Island")},
        {_T("AU"), _T("Australia")},
        {_T("AT"), _T("Austria")},
        {_T("AZ"), _T("Azerbaidjan")},
        {_T("BS"), _T("Bahamas")},
        {_T("BH"), _T("Bahrain")},
        {_T("BD"), _T("Bangladesh")},
        {_T("BB"), _T("Barbados")},
        {_T("BY"), _T("Belarus")},
        {_T("BE"), _T("Belgium")},
        {_T("BZ"), _T("Belize")},
        {_T("BJ"), _T("Benin")},
        {_T("BM"), _T("Bermuda")},
        {_T("BT"), _T("Bhutan")},
        {_T("BO"), _T("Bolivia")},
        {_T("BA"), _T("Bosnia-Herzegovina")},
        {_T("BW"), _T("Botswana")},
        {_T("BV"), _T("Bouvet Island")},
        {_T("BR"), _T("Brazil")},
        {_T("IO"), _T("British Indian Ocean Territory")},
        {_T("BN"), _T("Brunei Darussalam")},
        {_T("BG"), _T("Bulgaria")},
        {_T("BF"), _T("Burkina Faso")},
        {_T("BI"), _T("Burundi")},
        {_T("KH"), _T("Cambodia, Kingdom of")},
        {_T("CM"), _T("Cameroon")},
        {_T("CA"), _T("Canada")},
        {_T("CV"), _T("Cape Verde")},
        {_T("KY"), _T("Cayman Islands")},
        {_T("CF"), _T("Central African Republic")},
        {_T("TD"), _T("Chad")},
        {_T("CL"), _T("Chile")},
        {_T("CN"), _T("China")},
        {_T("CX"), _T("Christmas Island")},
        {_T("CC"), _T("Cocos (Keeling) Islands")},
        {_T("CO"), _T("Colombia")},
        {_T("KM"), _T("Comoros")},
        {_T("CG"), _T("Congo")},
        {_T("CD"), _T("Congo, The Democratic Republic of the")},
        {_T("CK"), _T("Cook Islands")},
        {_T("CR"), _T("Costa Rica")},
        {_T("HR"), _T("Croatia")},
        {_T("CU"), _T("Cuba")},
        {_T("CY"), _T("Cyprus")},
        {_T("CZ"), _T("Czech Republic")},
        {_T("DK"), _T("Denmark")},
        {_T("DJ"), _T("Djibouti")},
        {_T("DM"), _T("Dominica")},
        {_T("DO"), _T("Dominican Republic")},
        {_T("TP"), _T("East Timor")},
        {_T("EC"), _T("Ecuador")},
        {_T("EG"), _T("Egypt")},
        {_T("SV"), _T("El Salvador")},
        {_T("GQ"), _T("Equatorial Guinea")},
        {_T("ER"), _T("Eritrea")},
        {_T("EE"), _T("Estonia")},
        {_T("ET"), _T("Ethiopia")},
        {_T("FK"), _T("Falkland Islands")},
        {_T("FO"), _T("Faroe Islands")},
        {_T("FJ"), _T("Fiji")},
        {_T("FI"), _T("Finland")},
        {_T("CS"), _T("Former Czechoslovakia")},
        {_T("SU"), _T("Former USSR")},
        {_T("FR"), _T("France")},
        {_T("FX"), _T("France (European Territory)")},
        {_T("GF"), _T("French Guyana")},
        {_T("TF"), _T("French Southern Territories")},
        {_T("GA"), _T("Gabon")},
        {_T("GM"), _T("Gambia")},
        {_T("GE"), _T("Georgia")},
        {_T("DE"), _T("Germany")},
        {_T("GH"), _T("Ghana")},
        {_T("GI"), _T("Gibraltar")},
        {_T("GB"), _T("Great Britain")},
        {_T("GR"), _T("Greece")},
        {_T("GL"), _T("Greenland")},
        {_T("GD"), _T("Grenada")},
        {_T("GP"), _T("Guadeloupe (French)")},
        {_T("GU"), _T("Guam (USA)")},
        {_T("GT"), _T("Guatemala")},
        {_T("GG"), _T("Guernsey")},
        {_T("GN"), _T("Guinea")},
        {_T("GW"), _T("Guinea Bissau")},
        {_T("GY"), _T("Guyana")},
        {_T("HT"), _T("Haiti")},
        {_T("HM"), _T("Heard and McDonald Islands")},
        {_T("VA"), _T("Holy See (Vatican City State)")},
        {_T("HN"), _T("Honduras")},
        {_T("HK"), _T("Hong Kong")},
        {_T("HU"), _T("Hungary")},
        {_T("IS"), _T("Iceland")},
        {_T("IN"), _T("India")},
        {_T("ID"), _T("Indonesia")},
        {_T("IR"), _T("Iran")},
        {_T("IQ"), _T("Iraq")},
        {_T("IE"), _T("Ireland")},
        {_T("IM"), _T("Isle of Man")},
        {_T("IL"), _T("Israel")},
        {_T("IT"), _T("Italy")},
        {_T("CI"), _T("Ivory Coast (Cote D'Ivoire)")},
        {_T("JM"), _T("Jamaica")},
        {_T("JP"), _T("Japan")},
        {_T("JO"), _T("Jordan")},
        {_T("KZ"), _T("Kazakhstan")},
        {_T("KE"), _T("Kenya")},
        {_T("KI"), _T("Kiribati")},
        {_T("KW"), _T("Kuwait")},
        {_T("KG"), _T("Kyrgyz Republic (Kyrgyzstan)")},
        {_T("LA"), _T("Laos")},
        {_T("LV"), _T("Latvia")},
        {_T("LB"), _T("Lebanon")},
        {_T("LS"), _T("Lesotho")},
        {_T("LR"), _T("Liberia")},
        {_T("LY"), _T("Libya")},
        {_T("LI"), _T("Liechtenstein")},
        {_T("LT"), _T("Lithuania")},
        {_T("LU"), _T("Luxembourg")},
        {_T("MO"), _T("Macau")},
        {_T("MK"), _T("Macedonia")},
        {_T("MG"), _T("Madagascar")},
        {_T("MW"), _T("Malawi")},
        {_T("MY"), _T("Malaysia")},
        {_T("MV"), _T("Maldives")},
        {_T("ML"), _T("Mali")},
        {_T("MT"), _T("Malta")},
        {_T("MH"), _T("Marshall Islands")},
        {_T("MQ"), _T("Martinique (French)")},
        {_T("MR"), _T("Mauritania")},
        {_T("MU"), _T("Mauritius")},
        {_T("YT"), _T("Mayotte")},
        {_T("MX"), _T("Mexico")},
        {_T("FM"), _T("Micronesia")},
        {_T("MD"), _T("Moldavia")},
        {_T("MC"), _T("Monaco")},
        {_T("MN"), _T("Mongolia")},
        {_T("MS"), _T("Montserrat")},
        {_T("MA"), _T("Morocco")},
        {_T("MZ"), _T("Mozambique")},
        {_T("MM"), _T("Myanmar")},
        {_T("NA"), _T("Namibia")},
        {_T("NR"), _T("Nauru")},
        {_T("NP"), _T("Nepal")},
        {_T("NL"), _T("Netherlands")},
        {_T("AN"), _T("Netherlands Antilles")},
        {_T("NT"), _T("Neutral Zone")},
        {_T("NC"), _T("New Caledonia (French)")},
        {_T("NZ"), _T("New Zealand")},
        {_T("NI"), _T("Nicaragua")},
        {_T("NE"), _T("Niger")},
        {_T("NG"), _T("Nigeria")},
        {_T("NU"), _T("Niue")},
        {_T("NF"), _T("Norfolk Island")},
        {_T("KP"), _T("North Korea")},
        {_T("MP"), _T("Northern Mariana Islands")},
        {_T("NO"), _T("Norway")},
        {_T("OM"), _T("Oman")},
        {_T("PK"), _T("Pakistan")},
        {_T("PW"), _T("Palau")},
        {_T("PS"), _T("Palestinian Territories")},
        {_T("PA"), _T("Panama")},
        {_T("PG"), _T("Papua New Guinea")},
        {_T("PY"), _T("Paraguay")},
        {_T("PE"), _T("Peru")},
        {_T("PH"), _T("Philippines")},
        {_T("PN"), _T("Pitcairn Island")},
        {_T("PL"), _T("Poland")},
        {_T("PF"), _T("Polynesia (French)")},
        {_T("PT"), _T("Portugal")},
        {_T("PR"), _T("Puerto Rico")},
        {_T("QA"), _T("Qatar")},
        {_T("RE"), _T("Reunion (French)")},
        {_T("RO"), _T("Romania")},
        {_T("RU"), _T("Russian Federation")},
        {_T("RW"), _T("Rwanda")},
        {_T("GS"), _T("S. Georgia & S. Sandwich Isls.")},
        {_T("SH"), _T("Saint Helena")},
        {_T("KN"), _T("Saint Kitts & Nevis Anguilla")},
        {_T("LC"), _T("Saint Lucia")},
        {_T("PM"), _T("Saint Pierre and Miquelon")},
        {_T("ST"), _T("Saint Tome (Sao Tome) and Principe")},
        {_T("VC"), _T("Saint Vincent & Grenadines")},
        {_T("WS"), _T("Samoa")},
        {_T("SM"), _T("San Marino")},
        {_T("SA"), _T("Saudi Arabia")},
        {_T("SN"), _T("Senegal")},
        {_T("SC"), _T("Seychelles")},
        {_T("SL"), _T("Sierra Leone")},
        {_T("SG"), _T("Singapore")},
        {_T("SK"), _T("Slovak Republic")},
        {_T("SI"), _T("Slovenia")},
        {_T("SB"), _T("Solomon Islands")},
        {_T("SO"), _T("Somalia")},
        {_T("ZA"), _T("South Africa")},
        {_T("KR"), _T("South Korea")},
        {_T("ES"), _T("Spain")},
        {_T("LK"), _T("Sri Lanka")},
        {_T("SD"), _T("Sudan")},
        {_T("SR"), _T("Suriname")},
        {_T("SJ"), _T("Svalbard and Jan Mayen Islands")},
        {_T("SZ"), _T("Swaziland")},
        {_T("SE"), _T("Sweden")},
        {_T("CH"), _T("Switzerland")},
        {_T("SY"), _T("Syria")},
        {_T("TJ"), _T("Tadjikistan")},
        {_T("TW"), _T("Taiwan")},
        {_T("TZ"), _T("Tanzania")},
        {_T("TH"), _T("Thailand")},
        {_T("TG"), _T("Togo")},
        {_T("TK"), _T("Tokelau")},
        {_T("TO"), _T("Tonga")},
        {_T("TT"), _T("Trinidad and Tobago")},
        {_T("TN"), _T("Tunisia")},
        {_T("TR"), _T("Turkey")},
        {_T("TM"), _T("Turkmenistan")},
        {_T("TC"), _T("Turks and Caicos Islands")},
        {_T("TV"), _T("Tuvalu")},
        {_T("UG"), _T("Uganda")},
        {_T("UA"), _T("Ukraine")},
        {_T("AE"), _T("United Arab Emirates")},
        {_T("UK"), _T("United Kingdom")},
        {_T("US"), _T("United States")},
        {_T("UY"), _T("Uruguay")},
        {_T("UM"), _T("USA Minor Outlying Islands")},
        {_T("UZ"), _T("Uzbekistan")},
        {_T("VU"), _T("Vanuatu")},
        {_T("VE"), _T("Venezuela")},
        {_T("VN"), _T("Vietnam")},
        {_T("VG"), _T("Virgin Islands (British)")},
        {_T("VI"), _T("Virgin Islands (USA)")},
        {_T("WF"), _T("Wallis and Futuna Islands")},
        {_T("EH"), _T("Western Sahara")},
        {_T("YE"), _T("Yemen")},
        {_T("YU"), _T("Yugoslavia")},
        {_T("ZR"), _T("Zaire")},
        {_T("ZM"), _T("Zambia")},
        {_T("ZW"), _T("Zimbabwe")}    
};

    static inline uint_t countriesCount()
    {
        return sizeof(countries)/sizeof(countries[0]);
    }

    int getCountryCodesCount()
    {
        return countriesCount();
    }
    
    static bool compareCountryByFirstChar(const CountryArrayEntry& e1, const CountryArrayEntry& e2)
    {
        return e1.name[0] < e2.name[0];
    }
    
    // always return index value in [0, statesCount-1)   
    int getCountryIndexByFirstChar(char_t inChar)
    {
        // We assume every country name starts with uppercase
        const CountryArrayEntry* end = countries + countriesCount();
        CountryArrayEntry entry = {{}, {toUpper(inChar)}};
        const CountryArrayEntry* p = std::lower_bound(countries, end, entry, compareCountryByFirstChar);
        if (end == p)
            --p;
        return p - countries;
    }   
    
    const char_t* getCountryName(int pos)
    {
        assert(pos >= 0 && pos < countriesCount());
        return countries[pos].name;
    }
    
    const char_t* getCountryCode(int pos)
    {
        assert(pos >= 0 && pos < countriesCount());
        return countries[pos].abbrev;
    }

/*     
    void getInternationalCountryName(int pos, String& out)
    {   
        out = getCountryName(pos);
    }

    void getInternationalCountryCode(int pos, String& out) 
    {
        out = getCountryCode(pos);
    }
*/
    
} 
