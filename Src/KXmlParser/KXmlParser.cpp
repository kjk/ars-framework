/*
  Implementation of XmlPull XML parsing API. It's a port of Java's kXML 2
  (http://kxml.org, http://kobjects.dyndns.org/kobjects/auto?self=%23c0a80001000000f5ad6a6fb3)
*/

#include "KXmlParser.hpp"
#include <Text.hpp>

using namespace std;
using namespace KXml2;

#ifdef __MWERKS__

#pragma pcrelconstdata on
#pragma far_code
#pragma inline_bottom_up on 
//#pragma inline_depth(100)

#endif

#define MUSTDO

//from XmlPullParser
#define NO_NAMESPACE ""

/*Init TYPES (we don't have XmlPullParser.cpp)*/
const XmlPullParser::TypeDescription_t XmlPullParser::TYPES[XmlPullParser::typesArrayLength] = 
{
    "START_DOCUMENT",
    "END_DOCUMENT",
    "START_TAG",
    "END_TAG",
    "TEXT",
    "CDSECT",
    "ENTITY_REF",
    "IGNORABLE_WHITESPACE",
    "PROCESSING_INSTRUCTION",
    "COMMENT",
    "DOCDECL"
};

const char_t* XmlPullParser::FEATURE_PROCESS_NAMESPACES = "http://xmlpull.org/v1/doc/features.html#process-namespaces";

inline int KXmlParser::getEventType()
{
    return type_;
}

KXmlParser::KXmlParser():
    reader_(0)
{
    fRelaxed_ = false;    
    srcBuf_.resize(128);
    txtBuf_.resize(128);
    attributes_.resize(16);
    elementStack_.resize(16);
    nspStack_.resize(8);
    nspCounts_.resize(4);    
}

KXmlParser::~KXmlParser() {}

error_t KXmlParser::peek(int& ret, int pos)
{
    error_t error;

    while (pos >= peekCount_) 
    {
        int nw;

        if (srcBuf_.length() <= 1)
        {
            if((error=reader_->read(nw)) != eNoError)
                return error;
        }       
        else if (srcPos_ < srcCount_)
            nw = srcBuf_[srcPos_++];
        else {
            if((error=reader_->read(srcCount_, srcBuf_, 0, srcBuf_.length())) != eNoError)
                return error;
            if (srcCount_ <= 0)
                nw = -1;
            else
                nw = srcBuf_[0];

            srcPos_ = 1;
        }

        if (nw == '\r') {
            wasCR_ = true;
            peek_[peekCount_++] = '\n';
        }
        else {
            if (nw == '\n') {
                if (!wasCR_)
                    peek_[peekCount_++] = '\n';                     
            }
            else 
                peek_[peekCount_++] = nw;

            wasCR_ = false;
        }
    }
    ret = peek_[pos];
    return eNoError;
}

error_t KXmlParser::peekType(int& ret)
{
    error_t error;
    int     retChild;

    if((error=peek(retChild,0))!=eNoError)
        return error;

    switch (retChild) 
    {
        case -1 :
            ret = END_DOCUMENT;
            return eNoError;
        case '&' :
            ret = ENTITY_REF;
            return eNoError;
        case '<' :
            if((error=peek(retChild,1))!=eNoError)
                return error;
            switch (retChild) 
            {
                case '/' :
                    ret = END_TAG;
                    return eNoError;
                case '?' :
                case '!' :
                    ret = LEGACY;
                    return eNoError;
                default :
                    ret = START_TAG;
                    return eNoError;
            }
        default :
            ret = TEXT;
            return eNoError;
    }
}

error_t KXmlParser::read(const char_t c) 
{
    error_t error;
    int a;

    if ((error=read(a))!=eNoError)
        return error;

    if (a != c)
        return eExpectedCharNotEqual;

    return eNoError;
}

error_t KXmlParser::read(int& ret)
{
    error_t error;        
    int result;

    if (peekCount_ == 0)
    {
        if ((error=peek(result,0))!=eNoError)
            return error;
    }
    else 
    {
        result = peek_[0];
        peek_[0] = peek_[1];
    }
    peekCount_--;

    column_++;

    if (result == '\n') 
    {
        line_++;
        column_ = 1;
    }
        
    ret = result;
    return eNoError;
}

inline String KXmlParser::get(int pos)
{
        return String(txtBuf_, pos, txtPos_ - pos);
}

void KXmlParser::push(int c)
{
    isWhitespace_ &= (c <= ' ');

    if (txtPos_ == (int)txtBuf_.length()) 
        txtBuf_.resize((txtBuf_.size()*4)/3 + 4);
    txtBuf_[txtPos_++] = char_t(c);
}

error_t KXmlParser::skip()
{
    error_t error;    
    int     c;

    while (true) 
    {
        if ((error=peek(c,0))!=eNoError)
            return error;
        if (c > ' ' || c == -1)
            break;
        if ((error=read(c))!=eNoError)
            return error;
    }
    return eNoError;
}

error_t KXmlParser::pushEntity() {
    error_t error;
    int     c;

    if ((error=read(c))!=eNoError) // &
        return error;

    // find entity that is not correct. find: "& " and push &
    if ((error=peek(c,0))!=eNoError)
        return error;
    if (c == ' ')
    {
        push('&');
        return eNoError;
    }

    int pos = txtPos_;

    while (true) 
    {
        if ((error=read(c))!=eNoError) 
            return error;

        if (c == ';')
            break;

        if (c < 128 
            && (c < '0' || c > '9')
            && (c < 'a' || c > 'z')
            && (c < 'A' || c > 'Z')
            && c != '_' && c != '-' && c != '#')
        {
                if(!relaxed_)
                    return eUnterminatedEntityRef;
                if (c != -1) 
                    push(c);
                /*
                TODO: Think about sth like this:
                text: "Starsky &Hatch blah blah"
                we push "Starsky Hatch bla bla"
                we dont push & if its not before space "& " -> push('&')
                if after & is text "&blah blah" we dont put "&blah" -> "blah"
                if we want to put "&blah" -> "&blah" we should do this:
                
                String code = get(pos); //get all what we have pushed so far
                txtPos_ = pos;
                push('&');              //push & before it
                for(size_t i=0; i<code.length(); i++)
                    push(code[i]);      //push it one more time
                */
                return eNoError;
        }

        push(c);
    }

    String code = get(pos);
    txtPos_ = pos;
    if (token_ && type_ == ENTITY_REF)
        name_ = code;

    if ('#'==code[0])
     {
        const char_t* begin=code.data();
        const char_t* end=begin+code.length();
        long c;
        error_t err;
        if ('x'==*begin)
        {
            ++begin;
            err=ArsLexis::numericValue(begin, end, c, 16);
        }
        else
            err=ArsLexis::numericValue(begin, end, c);
        if (errNone!=err)
            if (relaxed_)
                c=1;
            else                
                return err;        
        push(c);
        return eNoError;
    }

    String result = resolveEntity(code);
    unresolved_ = result.empty();
    if (unresolved_)
    {
        if (!token_ && !relaxed_)
            return eUnresolvedCode;
    }
    else
    {
        for (size_t i = 0; i < result.length(); i++)
            push(result[i]);
    }
    return eNoError;
}

error_t KXmlParser::readName(String& ret) 
{
    error_t error;
    int pos = txtPos_;
    int c;
    if((error=peek(c,0))!=eNoError)
        return error;

    if ((c < 'a' || c > 'z')
        && (c < 'A' || c > 'Z')
        && c != '_'
        && c != ':'
        && c < 0x0c0)
        return eNameExpected;

    do
    {
        if ((error=read(c))!=eNoError)
            return error;
        push(c);
        if ((error=peek(c,0))!=eNoError)
            return error;
    } while ((c >= 'a' && c <= 'z')
        || (c >= 'A' && c <= 'Z')
        || (c >= '0' && c <= '9')
        || c == '_'
        || c == '-'
        || c == ':'
        || c == '.'
        || c >= 0x0b7);

    ret = get(pos);
    txtPos_ = pos;
    return eNoError;
}

error_t KXmlParser::pushText(int delimiter, bool resolveEntities)
{
    error_t error;
    int next;

    if ((error=peek(next,0))!=eNoError)
        return error;

    while (next != -1 && next != delimiter)
    { // covers eof, '<', '"'

        if (delimiter == ' ')
            if (next <= ' ' || next == '>')
                break;

        if (next == '&')
        {
            if (!resolveEntities)
                break;
            
            if ((error=pushEntity())!=eNoError)
                return error;
        }
        else if (next == '\n' && type_==START_TAG)
        {
            if ((error=read(next))!=eNoError)
                return error;
            push(' ');
        }
        else
        {
            if ((error=read(next))!=eNoError)
                return error;
            push(next);
        }
        
        if ((error=peek(next,0))!=eNoError)
            return error;
    }
    return eNoError;
}

error_t KXmlParser::getNamespaceCount(int& ret, int depth)
{
    if (depth > depth_)
        return eIndexOutOfBoundsException;
    ret = nspCounts_[depth];
    return eNoError; 
}

String KXmlParser::getNamespace(const String& prefix) 
{
    if (0==prefix.compare("xml"))
        return "http://www.w3.org/XML/1998/namespace";
    if (0==prefix.compare("xmlns"))
        return "http://www.w3.org/2000/xmlns/";

    int j;
    getNamespaceCount(j, depth_);  //It's 100% safe: depth_ <= depth_

    for (int i = (j << 1) - 2; i >= 0; i -= 2)
    {
        if (prefix.empty())
        {
            if (nspStack_[i].empty())
                return nspStack_[i + 1];
        }
        else if (0==prefix.compare(nspStack_[i]))
            return nspStack_[i + 1];
    }
    return String();
}

error_t KXmlParser::adjustNsp(bool& ret)
{
    bool any = false;

    for (int i = 0; i < attributeCount_ << 2; i += 4)
    {
        String attrName = attributes_[i + 2];
        String::size_type cut = attrName.find(':');
        String prefix;

        if (cut != String::npos)
        {
            prefix.assign(attrName, 0, cut);
            attrName.erase(0, cut + 1);
        }
        else if (0==attrName.compare("xmlns"))
        {
            prefix_.assign(attrName);
            attrName=_T("");
        }
        else
            continue;

        if (0!=prefix_.compare("xmlns"))
            any = true;
        else
        {
            int j = (nspCounts_[depth_]++) << 1;

            nspStack_.resize(j + 2);
            nspStack_[j] = attrName;
            nspStack_[j + 1] = attributes_[i + 3];

            if (!attrName.empty() && attributes_[i + 3].empty())
                return eIllegalEmptyNamespace;

            attributes_.erase(attributes_.begin()+i,attributes_.begin()+i+3);
            --attributeCount_;
            attributes_.resize((attributes_.end() - attributes_.begin()) + 4);
            i -= 4;
        }
    }

    if (any)
    {
        for (int i = (attributeCount_ << 2) - 4; i >= 0; i -= 4)
        {

            String attrName = attributes_[i + 2];
            String::size_type cut = attrName.find(':');

            if (cut == 0 && !relaxed_)
                return eIllegalAttributeName;
            else if (String::npos!=cut)
            {
                String attrPrefix(attrName, 0, cut);
                attrName.erase(0, cut + 1);
                String attrNs = getNamespace(attrPrefix);

                if (attrNs.empty() && !relaxed_)
                    return eUndefinedPrefix;

                attributes_[i] = attrNs;
                attributes_[i + 1] = attrPrefix;
                attributes_[i + 2] = attrName;

                if (!relaxed_)
                {
                    for (int j = (attributeCount_ << 2) - 4; j > i; j -= 4)
                        if (0==attrName.compare(attributes_[j + 2]) && 0==attrNs.compare(attributes_[j]))
                            return eDuplicateAttribute;
                }
            }
        }
    }

    String::size_type cut = name_.find(':');
    if (cut == 0 && !relaxed_)
        return eIllegalTagName;
    else if (String::npos!=cut)
    {
        prefix_.assign(name_, 0, cut);
        name_.erase(0, cut + 1);
    }

    nameSpace_ = getNamespace(prefix_);

    if (nameSpace_.empty())\
    {
        if (!prefix_.empty() && !relaxed_)
            return eUndefinedPrefix;
        nameSpace_ = NO_NAMESPACE;
    }

    ret = any;
    return eNoError;
}

error_t KXmlParser::parseStartTag(bool xmldecl)
{
    error_t error;
    int c, tmp;

    if (!xmldecl)
        if ((error=read(c))!=eNoError)
            return error;

    if ((error=readName(name_))!=eNoError)
        return error;

    attributeCount_ = 0;

    while (true) {
        if ((error=skip())!=eNoError)
            return error;

        if ((error=peek(c,0))!=eNoError)
            return error;

        if (xmldecl) {
            if (c == '?') {
                if ((error=read(c))!=eNoError)
                    return error;
                if ((error=read('>'))!=eNoError)
                    return error;

                return eNoError;
            }
        }
        else {
            if (c == '/') {
                degenerated_ = true;
                if ((error=read(tmp))!=eNoError)
                    return error;
                if ((error=skip())!=eNoError)
                    return error;
                if ((error=read('>'))!=eNoError)
                    return error;
                break;
            }

            if (c == '>' && !xmldecl) {
                if ((error=read(tmp))!=eNoError)
                    return error;
                break;
            }
        }

        if (c == -1)
            return eUnexpectedEof;

        String attrName;
        if ((error=readName(attrName))!=eNoError)
            return error;

        if (attrName.empty())
            return eAttrNameExpected;

        int tmpC;
        if ((error=peek(tmpC,0))!=eNoError)
            return error;
        bool fCanBeAttrWithNoValue=false;
        if (tmpC==' ' || tmpC=='>')
            fCanBeAttrWithNoValue=true;
        if ((error=skip())!=eNoError)
            return error;
        if ((error=peek(tmpC,0))!=eNoError)
            return error;
        if (tmpC!='=' && relaxed_ && fCanBeAttrWithNoValue)
        {
            int i = (attributeCount_++) << 2;
            attributes_.resize(i + 4);
            // These calls are redundant!            
            i+=2;
//            attributes_[i++] = "";
//            attributes_[i++] = ""; //NULL;
            attributes_[i++] = attrName;
//            attributes_[i] = "";
        }
        else
        {
            if ((error=read('='))!=eNoError)
                return error;
            if ((error=skip())!=eNoError)
                return error;
            int delimiter;
            if ((error=read(delimiter))!=eNoError)
                return error;

            int p = txtPos_;

            if (delimiter != '\'' && delimiter != '"') {
                if (!relaxed_)
                {
                    return eInvalidDelimiter;
                }
                /* to detect attributes without "
                   for example: bgcolor=eeaaaa 
                   delimiter = 'e'
                */
                if(delimiter != ' ')
                    push(delimiter);

                delimiter = ' ';
            }

            int i = (attributeCount_++) << 2;

            attributes_.resize(i + 4);
            i+=2;
//            attributes_[i++] = "";
//            attributes_[i++] = ""; //NULL;
            attributes_[i++] = attrName;

            if ((error=pushText(delimiter, true))!=eNoError)
                return error;

            attributes_[i] = get(p);
            txtPos_ = p;

            if (delimiter != ' ')
                if ((error=read(tmp))!=eNoError)// skip endquote
                    return error;
        }
    }

    int sp = depth_++ << 2;

    elementStack_.resize(sp + 4);
    elementStack_[sp + 3] = name_;

    nspCounts_.resize(depth_ + 4);
    nspCounts_[depth_] = nspCounts_[depth_ - 1];

    if (!relaxed_)
    {
        String attrTemp1, attrTemp2;
        for (int i = attributeCount_ - 1; i > 0; i--) {
            for (int j = 0; j < i; j++) {
                if ((error=getAttributeName(attrTemp1,i))!=eNoError)
                    return error;
                if ((error=getAttributeName(attrTemp2,j))!=eNoError)
                    return error;
                if (0==attrTemp1.compare(attrTemp2))
                    return eDuplicateAttribute;
            }
        }
    }

    if (processNsp_)
    {
        bool tempB;
        if ((error=adjustNsp(tempB))!=eNoError)// skip endquote
            return error;
    }
    else
        nameSpace_=_T("");

    elementStack_[sp] = nameSpace_;
    elementStack_[sp + 1] = prefix_;
    elementStack_[sp + 2] = name_;

    return eNoError;
}

error_t KXmlParser::parseEndTag() 
{
    error_t error;
    int     temp;
        
    if ((error=read(temp))!=eNoError) // '<'
        return error;
    if ((error=read(temp))!=eNoError) // '/'
        return error;

    if ((error=readName(name_))!=eNoError) 
        return error;
    if ((error=skip())!=eNoError) 
        return error;
    if ((error=read('>'))!=eNoError) 
        return error;

    int sp = (depth_ - 1) << 2;

    if (!relaxed_) {

        if (depth_ == 0)
            return eElementStackEmpty;

        if (name_!=elementStack_[sp + 3])
            return eExpectedDifferentName;
    }
    else
    {   
        String str1 = name_;
        String str2 = elementStack_[sp + 3];
        //TODO: str1, str2 toLower()
        MUSTDO
        for (int t=0; t<(int)str1.length(); t++)
            str1[t] = ArsLexis::toLower(str1[t]);
        for(int t=0; t<(int)str2.length(); t++)
            str2[t] = ArsLexis::toLower(str2[t]);

        if (depth_ == 0 || str1 != str2)
            return eNoError; //was just return, so no error
    }
    nameSpace_ = elementStack_[sp];
    prefix_ = elementStack_[sp + 1];
    name_ = elementStack_[sp + 2];
    return eNoError;
}

/** precondition: &lt! consumed */
error_t KXmlParser::parseDoctype(bool pushV) {
    error_t error;
    int nesting = 1;
    bool quoted = false;
    int i;

    while (true) {
        if ((error=read(i))!=eNoError)
            return error;
        
        switch (i) {

            case -1 :
                return eUnexpectedEof;

            case '\'' :
                quoted = !quoted;
                break;

            case '<' :
                if (!quoted)
                    nesting++;
                break;

            case '>' :
                if (!quoted) {
                    if ((--nesting) == 0)
                        return eNoError;
                }
                break;
        }
        if (pushV)
            push(i);
    }
}

error_t KXmlParser::parseLegacy(int& ret, bool pushV) {
    error_t error;
    String req;
    int term;
    int result;
    int prev = 0;
    int c;
    int tmp1,tmp2;

    if ((error=read(c))!=eNoError) // <
        return error;
    if ((error=read(c))!=eNoError)
        return error;

    if (c == '?') {
        if((error=peek(tmp1,0))!=eNoError)
            return error;
        if((error=peek(tmp2,1))!=eNoError)
            return error;

        if ((tmp1 == 'x' || tmp1 == 'X')
            && (tmp2 == 'm' || tmp2 == 'M')) {

            if (pushV) {
                push(tmp1);
                push(tmp2);
            }
            if((error=read(tmp1))!=eNoError)
                return error;
            if((error=read(tmp1))!=eNoError)
                return error;


            if((error=peek(tmp1,0))!=eNoError)
                return error;
            if((error=peek(tmp2,1))!=eNoError)
                return error;
            if ((tmp1 == 'l' || tmp1 == 'L') && tmp2 <= ' ') {

                if (line_ != 1 || column_ > 4)
                    return ePIMustNotStartWithXml;

                if((error=parseStartTag(true))!=eNoError)
                    return error;

                if (attributeCount_ < 1 || 0!=attributes_[2].compare("version"))
                    return eVersionExpected;

                version_.assign(attributes_[3]);

                int pos = 1;

                if (pos < attributeCount_ && 0==attributes_[2+4].compare("encoding"))  {
                    encoding_.assign(attributes_[3+4]);
                    pos++;                        
                }

                if (pos < attributeCount_ && 0==attributes_[4*pos+2].compare("standalone")) {
                    const String& st = attributes_[3+4*pos];
                    if (0==st.compare("yes")) 
                        standalone_ = true;
                    else if (0==st.compare("no")) 
                        standalone_ = false;
                    else 
                        return eIllegalStandaloneValue;
                    pos++;
                }

                if (pos != attributeCount_) 
                    return eIllegalXmldecl;

                isWhitespace_ = true;
                txtPos_ = 0;                    

                ret = XML_DECL;
                return eNoError;
            }
        }

        term = '?';
        result = PROCESSING_INSTRUCTION;
    }
    else if (c == '!') {
        if((error=peek(tmp1,0))!=eNoError)
            return error;
        if (tmp1 == '-') {
            result = COMMENT;
            req = "--";
            term = '-';
        }
        else if (tmp1 == '[') {
            result = CDSECT;
            req = "[CDATA[";
            term = ']';
            pushV = true;
        }
        else {
            result = DOCDECL;
            req = "DOCTYPE";
            term = -1;
        }
    }
    else {
        ret = -1;
        return eIllegalStartTag;
    }

    for (size_t i = 0; i < req.length(); i++)
    {
        if((error=read(req[i]))!=eNoError)
            return error;
    }

    
    if (result == DOCDECL)
    {
        if((error=parseDoctype(pushV))!=eNoError)
            return error;
    }
    else {
        while (true) {
            if((error=read(c))!=eNoError)
                return error;
            if (c == -1)
                return eUnexpectedEof;

            if (pushV)
                push(c);

            if((error=peek(tmp1,0))!=eNoError)
                return error;
            if((error=peek(tmp2,1))!=eNoError)
                return error;

            if ((term == '?' || c == term)
                && tmp1 == term
                && tmp2 == '>')
                break;

            prev = c;
        }

        if (term == '-' && prev == '-' && !relaxed_)
            return eIllegalCommentDelimiter;

        if((error=read(tmp1))!=eNoError)
            return error;
        if((error=read(tmp1))!=eNoError)
            return error;

        if (pushV && term != '?')
            txtPos_--;
    }
    ret = result;
    return eNoError;
}

/** 
* common base for next and nextToken. Clears the state, except from 
* txtPos and whitespace. Does not set the type variable */
error_t KXmlParser::nextImpl()
{
    error_t error;

    if (reader_ == 0)
        return eNoInputSpecified;

    if (type_ == END_TAG)
        depth_--;

    while (true)
    {
        attributeCount_ = -1;

        if (degenerated_)
        {
            degenerated_ = false;
            type_ = END_TAG;
            return eNoError;
        }

        prefix_=_T("");
        name_=_T("");
        nameSpace_=_T("");
        text_=_T("");

        if((error=peekType(type_))!=eNoError)
            return error;

        switch (type_)
        {
            case ENTITY_REF :
                if((error=pushEntity())!=eNoError)
                    return error;
                return eNoError;

            case START_TAG :
                if((error=parseStartTag(false))!=eNoError)
                    return error;
                return eNoError;

            case END_TAG :
                if((error=parseEndTag())!=eNoError)
                    return error;
                return eNoError;

            case END_DOCUMENT :
                return eNoError;

            case TEXT :
                if((error=pushText('<', !token_))!=eNoError)
                    return error;
                if (0==depth_) {
                    if (isWhitespace_)
                        type_ = IGNORABLE_WHITESPACE;
                    // make exception switchable for instances.chg... !!!!
                    //	else 
                    //    return eTextNotAllowedOutsideRootElement;  
                }
                return eNoError;

            default :
                if((error=parseLegacy(type_,token_))!=eNoError)
                    return error;
                if (type_ != XML_DECL)
                    return eNoError;
                    
        }
    }
}

/* PUBLIC */
error_t KXmlParser::setInput(Reader& reader)
{
    reader_ = &reader;
    line_ = 1;
    column_ = 0;
    type_ = START_DOCUMENT;
    name_=_T("");
    nameSpace_=_T("");
    degenerated_ = false;
    attributeCount_ = -1;
    encoding_=_T("");
    version_=_T("");
    standalone_ = false;
    wasCR_ = false;

    srcPos_ = 0;
    srcCount_ = 0;
    peekCount_ = 0;
    depth_ = 0;

    entityMap_.clear();
    return eNoError;
}

bool KXmlParser::isProp (const String& n1, bool prop, const String& n2) {
        //if (!n1.startsWith("http://xmlpull.org/v1/doc/")) return false;
        if(n1.compare(0, 26, "http://xmlpull.org/v1/doc/")!=0) return false;
        if (prop) 
            return 0==n1.compare(42, String::npos, n2);
        else 
            return 0==n1.compare(40, String::npos, n2);
}

error_t KXmlParser::setFeature(const String& feature, bool flag)
{
    if (FEATURE_PROCESS_NAMESPACES == feature)
        processNsp_ = flag;
    else
        if (isProp(feature, false, "relaxed"))
            relaxed_ = flag;
        else
            return eUnsupportedFeature;

    return eNoError;
}

error_t KXmlParser::nextToken(int& ret)
{
    error_t error;
    
    isWhitespace_ = true;
    txtPos_ = 0;
    token_ = true;
    if ((error=nextImpl())!=eNoError)
        return error;
    ret = type_;
    return eNoError;
}

error_t KXmlParser::next(int& ret)
{
    error_t error;
    int tempT;
    txtPos_ = 0;
    isWhitespace_ = true;
    token_ = false;
    int minType = 9999;

    do {
        if ((error=nextImpl())!=eNoError)
            return error;

        if (type_ < minType)
            minType = type_;
        //	    if (curr <= TEXT) type = curr;  //was commented in java
        if ((error=peekType(tempT))!=eNoError)
            return error;
    }
    while (minType > ENTITY_REF // ignorable
        || (minType >= TEXT && tempT >= TEXT));

    type_ = minType;
    if (type_ > TEXT)
        type_ = TEXT;

    ret = type_;
    return eNoError;
}

bool KXmlParser::getFeature(const String& feature) 
{
    if (XmlPullParser::FEATURE_PROCESS_NAMESPACES == feature)
        return processNsp_;
    else 
        if (isProp(feature, false, "relaxed"))
            return relaxed_;
        else
            return false;
}

inline String KXmlParser::getInputEncoding() 
{
    return encoding_;
}

inline String KXmlParser::getNamespacePrefix(int pos) 
{
    return nspStack_[pos << 1];
}

inline String KXmlParser::getNamespaceUri(int pos) 
{
    return nspStack_[(pos << 1) + 1];
}

inline int KXmlParser::getDepth() 
{
    return depth_;
}

inline int KXmlParser::getLineNumber() 
{
    return line_;
}

inline int KXmlParser::getColumnNumber() 
{
    return column_;
}

error_t KXmlParser::isWhitespace(bool& ret)
{
    if (type_ != TEXT && type_ != IGNORABLE_WHITESPACE && type_ != CDSECT)
        return eIllegalType;
    ret = isWhitespace_;
    return eNoError;
}

inline String KXmlParser::getNamespace() 
{
    return nameSpace_;
}

inline String KXmlParser::getName() 
{
    return name_;
}

inline String KXmlParser::getPrefix() 
{
    return prefix_;
}

error_t KXmlParser::isEmptyElementTag(bool& ret)
{
    if (type_ != START_TAG)
        return eIllegalType;
    ret = degenerated_;
    return eNoError;
}

inline int KXmlParser::getAttributeCount() 
{
    return attributeCount_;
}

error_t KXmlParser::getAttributeNamespace(String& ret, int index) 
{
    if (index >= attributeCount_)
        return eIndexOutOfBoundsException;
    ret = attributes_[(index << 2)];
    return eNoError;
}

error_t KXmlParser::getAttributeName(String& ret, int index) 
{
    if (index >= attributeCount_)
        return eIndexOutOfBoundsException;
    ret = attributes_[(index << 2) + 2];
    return eNoError;
}

error_t KXmlParser::getAttributePrefix(String& ret, int index) 
{
    if (index >= attributeCount_)
        return eIndexOutOfBoundsException;
    ret = attributes_[(index << 2) + 1];
    return eNoError;
}

error_t KXmlParser::getAttributeValue(String& ret, int index) 
{
    if (index >= attributeCount_)
        return eIndexOutOfBoundsException;
    ret = attributes_[(index << 2) + 3];
    return eNoError;
}

String KXmlParser::getAttributeValue(const String& nameSpace, const String& name) 
{
    for (int i = (attributeCount_ << 2) - 4; i >= 0; i -= 4) 
    {
        if (0==attributes_[i + 2].compare(name) && (nameSpace.empty() || 0==attributes_[i].compare(nameSpace)))
                return attributes_[i + 3];
    }                                    
    return String();
}

error_t KXmlParser::nextTag(int& ret) 
{
    error_t error;
    int i;
    
    if ((error=next(i))!=eNoError)
        return error;
    if (type_ == TEXT && isWhitespace_)
        if((error=next(i))!=eNoError)
            return error;

    if (type_ != END_TAG && type_ != START_TAG)
        return eUnexpectedType;

    ret = type_;
    return eNoError;
}

error_t KXmlParser::nextText(String& ret)
{
    error_t error;
    int i;
    if (type_ != START_TAG)
        return ePreconditionStartTag;

    if((error=next(i))!=eNoError)
        return error;

    String result;

    if (type_ == TEXT) {
        result = getText();
        if ((error=next(i))!=eNoError)
            return error;
    }
    else
        result=_T("");

    if (type_ != END_TAG)
        return eEndTagExpected;

    ret = result;
    return eNoError;
}

String KXmlParser::getText()
{
    if ( (type_ < TEXT) || (type_ == ENTITY_REF && unresolved_))
        return String();
    else
        return get(0);
}

error_t KXmlParser::getPositionDescription(String& ret)
{
    String buf;

    if (type_ < typesArrayLength)
        buf = TYPES[type_];
    else
        buf = "unknown";

    buf += ' ';

    if (type_ == START_TAG || type_ == END_TAG)
    {
        if (degenerated_)
            buf += "(empty) ";
        buf += '<';
        if (type_ == END_TAG)
            buf += '/';

        if (prefix_ != "")
            buf.append(1, '{').append(nameSpace_).append(1, '}').append(prefix_).append(1, ':');
        buf += name_;

        int cnt = attributeCount_ << 2;
        for (int i = 0; i < cnt; i += 4)
        {
            buf += ' ';
            if (attributes_[i + 1] != "")
                buf.append(1, '{').append(attributes_[i]).append(1, '}').append(attributes_[i + 1]).append(1, ':');
            buf.append(attributes_[i + 2]).append("='").append(attributes_[i + 3]).append(1, '\'');
        }
        buf += '>';
    }
    else if (type_ == IGNORABLE_WHITESPACE)
    {
        // do nothing
    }
    else if (type_ != TEXT)
    {
        String text = getText();
        if ( ((ENTITY_REF == type_) || (END_DOCUMENT == type_)) && (text.empty()))
            text = "null";

       buf += text;
    }
    else if (isWhitespace_)
        buf += "(whitespace)";
    else
    {
        String text = getText();
        if (text.length() > 16)
        {
            text.resize(16);
            text.append("...");
        }            
        buf.append(text);            
    }

    buf.append(" @");
    char_t numBuffer[32];
    int len=tprintf(numBuffer, _T("%d:%d"), line_, column_);
    if (len>0)
        buf.append(numBuffer, len);
    ret = buf;
    return eNoError;
}

// TODO: is this the right interface? What to do in case of not finding the substitution
String KXmlParser::resolveEntity(const String& entity)
{
    // first resolve standard entities
    // TODO: could be optimized further by doing case on first letter first
    MUSTDO
    if (0==entity.compare("amp")) return "&";
    if (0==entity.compare("apos")) return "'";
    if (0==entity.compare("gt")) return ">";
    if (0==entity.compare("lt")) return "<";
    if (0==entity.compare("quot")) return "\"";

    EntityMap_t::const_iterator it=entityMap_.find(entity);
    if (it==entityMap_.end())
        return _T("");
    return (*it).second;
}

void KXmlParser::defineEntityReplacementText(const String& entity, const String& value)
{
    // TODO: should we check if a given replacement already exists?
    MUSTDO
    entityMap_[entity]=value;
}
