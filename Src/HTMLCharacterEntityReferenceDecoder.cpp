#include "HTMLCharacterEntityReferenceDecoder.hpp"

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
        insert("agrave", 192);
        insert("aacute", 193);
        insert("acirc", 194);
        insert("atilde", 195);
        insert("auml", 196);
        insert("aring", 197);
        insert("aelig", 198);
        insert("ccedil", 199);
        insert("egrave", 200);
        insert("eacute", 201);
        insert("ecirc", 202);
        insert("euml", 203);
        insert("igrave", 204);
        insert("iacute", 205);
        insert("icirc", 206);
        insert("iuml", 207);
        insert("eth", 208);
        insert("ntilde", 209);
        insert("ograve", 210);
        insert("oacute", 211);
        insert("ocirc", 212);
        insert("otilde", 213);
        insert("ouml", 214);
        insert("times", 215);
        insert("oslash", 216);
        insert("ugrave", 217);
        insert("uacute", 218);
        insert("ucirc", 219);
        insert("uuml", 220);
        insert("yacute", 221);
        insert("thorn", 222);
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