#include <HTMLCharacterEntityReferenceDecoder.hpp>

namespace ArsLexis 
{

    void HTMLCharacterEntityReferenceDecoder::insert(const char* key, unsigned char value)
    {
        map_[key]=value;
    }

    HTMLCharacterEntityReferenceDecoder::HTMLCharacterEntityReferenceDecoder()
    {
        // This is machine generated code. Function insert() is used because it generates 50% smaller code than inline version.
        // Nevertheless this code still sucks and probably we should use gperf to make it suck less...
        insert("quot", 34);
        insert("amp", 38);
        insert("lt", 60);
        insert("gt", 62);
        insert("nbsp", 160);
        insert("iexcl", 161);
        insert("cent", 162);
        insert("pound", 163);
        insert("curren", 164);
        insert("yen", 165);
        insert("brvbar", 166);
        insert("sect", 167);
        insert("uml", 168);
        insert("copy", 169);
        insert("ordf", 170);
        insert("laquo", 171);
        insert("not", 172);
        insert("shy", 173);
        insert("reg", 174);
        insert("macr", 175);
        insert("deg", 176);
        insert("plusmn", 177);
        insert("sup2", 178);
        insert("sup3", 179);
        insert("acute", 180);
        insert("micro", 181);
        insert("para", 182);
        insert("middot", 183);
        insert("cedil", 184);
        insert("sup1", 185);
        insert("ordm", 186);
        insert("raquo", 187);
        insert("frac14", 188);
        insert("frac12", 189);
        insert("frac34", 190);
        insert("iquest", 191);
        insert("Agrave", 192);
        insert("Aacute", 193);
        insert("Acirc", 194);
        insert("Atilde", 195);
        insert("Auml", 196);
        insert("Aring", 197);
        insert("AElig", 198);
        insert("Ccedil", 199);
        insert("Egrave", 200);
        insert("Eacute", 201);
        insert("Ecirc", 202);
        insert("Euml", 203);
        insert("Igrave", 204);
        insert("Iacute", 205);
        insert("Icirc", 206);
        insert("Iuml", 207);
        insert("ETH", 208);
        insert("Ntilde", 209);
        insert("Ograve", 210);
        insert("Oacute", 211);
        insert("Ocirc", 212);
        insert("Otilde", 213);
        insert("Ouml", 214);
        insert("times", 215);
        insert("Oslash", 216);
        insert("Ugrave", 217);
        insert("Uacute", 218);
        insert("Ucirc", 219);
        insert("Uuml", 220);
        insert("Yacute", 221);
        insert("THORN", 222);
        insert("szlig", 223);
        insert("agrave", 224);
        insert("aacute", 225);
        insert("acirc", 226);
        insert("atilde", 227);
        insert("auml", 228);
        insert("aring", 229);
        insert("aelig", 230);
        insert("ccedil", 231);
        insert("egrave", 232);
        insert("eacute", 233);
        insert("ecirc", 234);
        insert("euml", 235);
        insert("igrave", 236);
        insert("iacute", 237);
        insert("icirc", 238);
        insert("iuml", 239);
        insert("eth", 240);
        insert("ntilde", 241);
        insert("ograve", 242);
        insert("oacute", 243);
        insert("ocirc", 244);
        insert("otilde", 245);
        insert("ouml", 246);
        insert("divide", 247);
        insert("oslash", 248);
        insert("ugrave", 249);
        insert("uacute", 250);
        insert("ucirc", 251);
        insert("uuml", 252);
        insert("yacute", 253);
        insert("thorn", 254);
        insert("yuml", 255);
        // Note: these mappings below are formally incorrect, but often used and we should try to display them anyway
        insert("ndash", 150); // 8211
        insert("mdash", 151); // 8212
    }

    char HTMLCharacterEntityReferenceDecoder::decode(const char* entityReference) const
    {
        char result=chrNull;
        Map_t::const_iterator it=map_.find(entityReference);
        if (it!=map_.end())
            result=it->second;
        return result;
    }

}