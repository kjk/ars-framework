/*
  Implementation of XmlPull XML parsing API. It's a port of Java's kXML 2
  (http://kxml.org, http://kobjects.dyndns.org/kobjects/auto?self=%23c0a80001000000f5ad6a6fb3)
*/

#include "KxmlParser.hpp"

using namespace std;
using namespace KXml2;

//TODO: all with this (when this line is commented)
#define MUSTDO ;
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>

//from XmlPullParser
#define NO_NAMESPACE ""

/*Init TYPES (we don't have XmlPullParser.cpp)*/
const String XmlPullParser::TYPES[] = 
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
#define TYPESLENGHT 11

String XmlPullParser::FEATURE_PROCESS_NAMESPACES = "http://xmlpull.org/v1/doc/features.html#process-namespaces";

KXmlParser::KXmlParser()
{
    fRelaxed_ = false;    
    entityMap_ = NULL;   
    reader_ = NULL;

    srcBuf_ = String(128,' ');
    txtBuf_ = String(128,' ');

    ensureCapacity(attributes_, 16);
    ensureCapacity(elementStack_, 16);
    ensureCapacity(nspStack_, 8);
    ensureCapacityInt(nspCounts_, 4);
}

KXmlParser::~KXmlParser() {}

/* PRIVATE */
void KXmlParser::ensureCapacity(vector<String>& vect, int size)
{
    vect.resize(size,"");
}

void KXmlParser::ensureCapacityInt(vector<int>& vect, int size)
{
    vect.resize(size,0);
}

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

error_t KXmlParser::read(const char c) 
{
    error_t error;
    int a;

    if((error=read(a))!=eNoError)
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
        if((error=peek(result,0))!=eNoError)
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

String KXmlParser::get(int pos)
{
        return txtBuf_.substr (pos, txtPos_ - pos);
}

void KXmlParser::push(int c)
{
    isWhitespace_ &= c <= ' ';

    if (txtPos_ == (int)txtBuf_.length()) 
        txtBuf_.resize((txtBuf_.size()*4)/3 + 4);
    txtBuf_[txtPos_++] = (char) c;
}

error_t KXmlParser::skip()
{
    error_t error;    
    int     c;

    while (true) 
    {
        if((error=peek(c,0))!=eNoError)
            return error;
        if (c > ' ' || c == -1)
            break;
        if((error=read(c))!=eNoError)
            return error;
    }
    return eNoError;
}

error_t KXmlParser::pushEntity() {
    error_t error;
    int     c;

    if((error=read(c))!=eNoError) // &
        return error;

    int pos = txtPos_;

    while (true) 
    {
        if((error=read(c))!=eNoError) 
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
                return eNoError;
        }

        push(c);
    }

    String code = get(pos);
    txtPos_ = pos;
    if (token_ && type_ == ENTITY_REF)
        name_ = code;

    if (code[0] == '#')
     {
        char_t *endPtr;

        int c =
            (code[1] == 'x'
                ? (int) strtol(&code[2],&endPtr, 16)
                : atoi(&code[1]));
        push(c);
        return eNoError;
    }

    String result = resolveEntity(code);

    unresolved_ = result == "";

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
        if((error=read(c))!=eNoError)
            return error;
        push(c);
        if((error=peek(c,0))!=eNoError)
            return error;
    } while ((c >= 'a' && c <= 'z')
        || (c >= 'A' && c <= 'Z')
        || (c >= '0' && c <= '9')
        || c == '_'
        || c == '-'
        || c == ':'
        || c == '.'
        || c >= 0x0b7);

    String result = get(pos);
    txtPos_ = pos;
    ret = result;
    return eNoError;
}

error_t KXmlParser::pushText(int delimiter, bool resolveEntities)
{
    error_t error;
    int next;

    if((error=peek(next,0))!=eNoError)
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
            
            if((error=pushEntity())!=eNoError)
                return error;
        }
        else if (next == '\n' && type_==START_TAG)
        {
            if((error=read(next))!=eNoError)
                return error;
            push(' ');
        }
        else
        {
            if((error=read(next))!=eNoError)
                return error;
            push(next);
        }
        
        if((error=peek(next,0))!=eNoError)
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

String KXmlParser::getNamespace(String prefix) 
{
    if ("xml" == prefix)
        return "http://www.w3.org/XML/1998/namespace";
    if ("xmlns" == prefix)
        return "http://www.w3.org/2000/xmlns/";

    int j;
    getNamespaceCount(j, depth_);  //It's 100% safe: depth_ <= depth_

    for (int i = (j << 1) - 2; i >= 0; i -= 2)
    {
        if (prefix == "")
        {
            if (nspStack_[i] == "")
                return nspStack_[i + 1];
        }
        else if (prefix == nspStack_[i])
            return nspStack_[i + 1];
    }
    return "";
}

error_t KXmlParser::adjustNsp(bool& ret)
{
    bool any = false;

    for (int i = 0; i < attributeCount_ << 2; i += 4)
    {
        String attrName = attributes_[i + 2];
        int cut = attrName.find(':');
        String prefix;

        if (cut != -1)
        {
            prefix = attrName.substr(0, cut);
            attrName = attrName.substr(cut + 1);
        }
        else if (attrName == "xmlns")
        {
            prefix_ = attrName;
            attrName = "";
        }
        else
            continue;

        if (prefix_ != "xmlns" )
            any = true;
        else
        {
            int j = (nspCounts_[depth_]++) << 1;

            ensureCapacity(nspStack_, j + 2);
            nspStack_[j] = attrName;
            nspStack_[j + 1] = attributes_[i + 3];

            if (attrName != "" && attributes_[i + 3] == "")
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
            int cut = attrName.find(':');

            if (cut == 0 && !relaxed_)
                return eIllegalAttributeName;
            else if (cut != -1)
            {
                String attrPrefix = attrName.substr(0, cut);

                attrName = attrName.substr(cut + 1);

                String attrNs = getNamespace(attrPrefix);

                if (attrNs == "" && !relaxed_)
                    return eUndefinedPrefix;

                attributes_[i] = attrNs;
                attributes_[i + 1] = attrPrefix;
                attributes_[i + 2] = attrName;

                if (!relaxed_)
                {
                    for (int j = (attributeCount_ << 2) - 4; j > i; j -= 4)
                        if (attrName == attributes_[j + 2]
                            && attrNs == attributes_[j])
                        {
                            return eDuplicateAttribute;
                        }
                }
            }
        }
    }

    int cut = name_.find(':');

    if (cut == 0 && !relaxed_)
        return eIllegalTagName;
    else if (cut != -1)
    {
        prefix_ = name_.substr(0, cut);
        name_ = name_.substr(cut + 1);
    }

    nameSpace_ = getNamespace(prefix_);

    if (nameSpace_ == "")\
    {
        if (prefix_ != "" && !relaxed_)
            return eUndefinedPrefix;
        nameSpace_ = NO_NAMESPACE;
    }

    ret = any;
    return eNoError;
}

error_t KXmlParser::parseStartTag(bool xmldecl)
{
    error_t error;
    int c,tmp;

    if (!xmldecl)
        if((error=read(c))!=eNoError)
            return error;

    if((error=readName(name_))!=eNoError)
        return error;

    attributeCount_ = 0;

    while (true) {
        if((error=skip())!=eNoError)
            return error;

        if((error=peek(c,0))!=eNoError)
            return error;

        if (xmldecl) {
            if (c == '?') {
                if((error=read(c))!=eNoError)
                    return error;
                if((error=read('>'))!=eNoError)
                    return error;

                return eNoError;
            }
        }
        else {
            if (c == '/') {
                degenerated_ = true;
                if((error=read(tmp))!=eNoError)
                    return error;
                if((error=skip())!=eNoError)
                    return error;
                if((error=read('>'))!=eNoError)
                    return error;
                break;
            }

            if (c == '>' && !xmldecl) {
                if((error=read(tmp))!=eNoError)
                    return error;
                break;
            }
        }

        if (c == -1)
            return eUnexpectedEof;

        String attrName;
        if((error=readName(attrName))!=eNoError)
            return error;

        if (attrName.length() == 0)
            return eAttrNameExpected;

        int tmpC;
        if((error=peek(tmpC,0))!=eNoError)
            return error;
        bool fCanBeAttrWithNoValue=false;
        if (tmpC==' ' || tmpC=='>')
            fCanBeAttrWithNoValue=true;
        if((error=skip())!=eNoError)
            return error;
        if((error=peek(tmpC,0))!=eNoError)
            return error;
        if (tmpC!='=' && relaxed_ && fCanBeAttrWithNoValue)
        {
            int i = (attributeCount_++) << 2;

            ensureCapacity(attributes_, i + 4);

            attributes_[i++] = "";
            attributes_[i++] = ""; //NULL;
            attributes_[i++] = attrName;
            attributes_[i] = "";
        }
        else
        {
            if((error=read('='))!=eNoError)
                return error;
            if((error=skip())!=eNoError)
                return error;
            int delimiter;
            if((error=read(delimiter))!=eNoError)
                return error;

            if (delimiter != '\'' && delimiter != '"') {
                if (!relaxed_)
                {
                    return eInvalidDelimiter;
                }
                delimiter = ' ';
            }

            int i = (attributeCount_++) << 2;

            ensureCapacity(attributes_, i + 4);

            attributes_[i++] = "";
            attributes_[i++] = ""; //NULL;
            attributes_[i++] = attrName;

            int p = txtPos_;
            if((error=pushText(delimiter, true))!=eNoError)
                return error;

            attributes_[i] = get(p);
            txtPos_ = p;

            if (delimiter != ' ')
                if((error=read(tmp))!=eNoError)// skip endquote
                    return error;
        }
    }

    int sp = depth_++ << 2;

    ensureCapacity(elementStack_, sp + 4);
    elementStack_[sp + 3] = name_;

    ensureCapacityInt(nspCounts_ , depth_ + 4);
    nspCounts_[depth_] = nspCounts_[depth_ - 1];

    if (!relaxed_)
    {
        String attrTemp1, attrTemp2;
        for (int i = attributeCount_ - 1; i > 0; i--) {
            for (int j = 0; j < i; j++) {
                if((error=getAttributeName(attrTemp1,i))!=eNoError)
                    return error;
                if((error=getAttributeName(attrTemp2,j))!=eNoError)
                    return error;
                if (attrTemp1 == attrTemp2)
                    return eDuplicateAttribute;
            }
        }
    }

    if (processNsp_)
    {
        bool tempB;
        if((error=adjustNsp(tempB))!=eNoError)// skip endquote
            return error;
    }
    else
        nameSpace_ = "";

    elementStack_[sp] = nameSpace_;
    elementStack_[sp + 1] = prefix_;
    elementStack_[sp + 2] = name_;

    return eNoError;
}

error_t KXmlParser::parseEndTag() 
{
    error_t error;
    int     temp;
        
    if((error=read(temp))!=eNoError) // '<'
        return error;
    if((error=read(temp))!=eNoError) // '/'
        return error;

    if((error=readName(name_))!=eNoError) 
        return error;
    if((error=skip())!=eNoError) 
        return error;
    if((error=read('>'))!=eNoError) 
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
        for(int t=0;t<(int)str1.length();t++)
            str1[t] = (char_t)tolower((char_t) str1[t]);
        for(int t=0;t<(int)str2.length();t++)
            str2[t] = (char_t)tolower((char_t) str2[t]);

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
            if((error=read(i))!=eNoError)
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
    String req = "";
    int term;
    int result;
    int prev = 0;
    int c;
    int tmp1,tmp2;

    if((error=read(c))!=eNoError) // <
        return error;
    if((error=read(c))!=eNoError)
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

                if (attributeCount_ < 1 || "version"!=attributes_[2])
                    return eVersionExpected;

                version_ = attributes_[3];

                int pos = 1;

                if (pos < attributeCount_ && "encoding"==attributes_[2+4])  {
                    encoding_ = attributes_[3+4];
                    pos++;                        
                }

                if (pos < attributeCount_ && "standalone"==attributes_[4*pos+2]) {
                    String st = attributes_[3+4*pos];
                    if ("yes"==st) 
                        standalone_ = true;
                    else if ("no"==st) 
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

    if (reader_ == NULL)
        return eNoInputSpecified;

    if (type_ == END_TAG)
        depth_--;

    while(true)
    {
        attributeCount_ = -1;

        if (degenerated_)
        {
            degenerated_ = false;
            type_ = END_TAG;
            return eNoError;
        }

        prefix_ = "";
        name_ = "";
        nameSpace_ = "";
        text_ = "";

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
                if (depth_ == 0) {
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
error_t KXmlParser::setInput(XmlReader *reader)
{
    reader_ = reader;
    line_ = 1;
    column_ = 0;
    type_ = START_DOCUMENT;
    name_ = "";
    nameSpace_ = "";
    degenerated_ = false;
    attributeCount_ = -1;
    encoding_ = "";
    version_ = "";
    standalone_ = false;
    wasCR_ = false;
    if (reader_ == NULL)
        return eNoReader;
 
    srcPos_ = 0;
    srcCount_ = 0;
    peekCount_ = 0;
    depth_ = 0;

    if (entityMap_ != NULL)
    {
        entityMap_->Dispose();
        delete entityMap_;
        entityMap_ = NULL;
    }
    return eNoError;
}

bool KXmlParser::isProp (String n1, bool prop, String n2) {
        //if (!n1.startsWith("http://xmlpull.org/v1/doc/")) return false;
        if(n1.compare(0, 26, "http://xmlpull.org/v1/doc/")!=0) return false;
        if (prop) 
            return n1.substr(42) == n2;            
        else 
            return n1.substr(40) == n2;
}

error_t KXmlParser::setFeature(String feature, bool flag)
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
        if((error=nextImpl())!=eNoError)
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
        if((error=nextImpl())!=eNoError)
            return error;

        if (type_ < minType)
            minType = type_;
        //	    if (curr <= TEXT) type = curr;  //was commented in java
        if((error=peekType(tempT))!=eNoError)
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

bool KXmlParser::getFeature(String feature) 
{
    if (XmlPullParser::FEATURE_PROCESS_NAMESPACES == feature)
        return processNsp_;
    else 
        if (isProp(feature, false, "relaxed"))
            return relaxed_;
        else
            return false;
}

String KXmlParser::getInputEncoding() 
{
    return encoding_;
}

String KXmlParser::getNamespacePrefix(int pos) 
{
    return nspStack_[pos << 1];
}

String KXmlParser::getNamespaceUri(int pos) 
{
    return nspStack_[(pos << 1) + 1];
}

int KXmlParser::getDepth() 
{
    return depth_;
}

int KXmlParser::getLineNumber() 
{
    return line_;
}

int KXmlParser::getColumnNumber() 
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

String KXmlParser::getNamespace() 
{
    return nameSpace_;
}

String KXmlParser::getName() 
{
    return name_;
}

String KXmlParser::getPrefix() 
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

int KXmlParser::getAttributeCount() 
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

String KXmlParser::getAttributeValue(String nameSpace, String name) 
{
    for (int i = (attributeCount_ << 2) - 4; i >= 0; i -= 4) 
    {
        if (attributes_[i + 2] == name && (nameSpace == "" || attributes_[i] == nameSpace))
                return attributes_[i + 3];
    }                                    
    return "";
}

error_t KXmlParser::nextTag(int& ret) 
{
    error_t error;
    int i;
    
    if((error=next(i))!=eNoError)
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
        if((error=next(i))!=eNoError)
            return error;
    }
    else
        result = "";

    if (type_ != END_TAG)
        return eEndTagExpected;

    ret = result;
    return eNoError;
}

String KXmlParser::getText()
{
    if ( (type_ < TEXT) || (type_ == ENTITY_REF && unresolved_))
        return "";
    else
        return get(0);
}

error_t KXmlParser::getPositionDescription(String& ret)
{
    String buf;

    if (type_ < TYPESLENGHT)
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
            buf += "{" + nameSpace_ + "}" + prefix_ + ":";
        buf += name_;

        int cnt = attributeCount_ << 2;
        for (int i = 0; i < cnt; i += 4)
        {
            buf += ' ';
            if (attributes_[i + 1] != "")
                buf += "{" + attributes_[i] + "}" + attributes_[i + 1] + ":";
            buf += attributes_[i + 2] + "='" + attributes_[i + 3] + "'";
        }

        buf += '>';
    }
    else if (type_ == IGNORABLE_WHITESPACE);
    else if (type_ != TEXT)
    {
        String text = getText();
        if ( ((ENTITY_REF == type_) || (END_DOCUMENT == type_)) && (""==text))
            text = "null";

       buf += text;
    }
    else if (isWhitespace_)
        buf += "(whitespace)";
    else
    {
        String text = getText();
        if (text.length() > 16)
            text = text.substr(0, 16) + "...";
        buf.append(text);            
    }

    MUSTDO //better version

    std::stringstream tempBuf;
    String tempStr;

    buf += " @";
    tempBuf << line_ << ":" << column_;
    tempBuf >> tempStr;
    buf += tempStr;
    //buf += ":";
    //buf << (int)column_;
    
    ret = buf;
    return eNoError;
}

int KXmlParser::getEventType()
{
    return type_;
}

// TODO: is this the right interface? What to do in case of not finding the substitution
String KXmlParser::resolveEntity(String entity)
{
    // first resolve standard entities
    // TODO: could be optimized further by doing case on first letter first
    MUSTDO
    if (entity=="amp") return "&";
    if (entity=="apos") return "'";
    if (entity=="gt") return ">";
    if (entity=="lt") return "<";
    if (entity=="quot") return "\"";

    if (NULL==entityMap_)
    {
        return ""; // TODO: an exception? //RE TODO: Why? We dont need to throw exception
    }

    String value = entityMap_->get(entity);
    return value;
}

void KXmlParser::defineEntityReplacementText(String entity, String value)
{
    // TODO: should we check if a given replacement already exists?
    MUSTDO
    if (NULL==entityMap_)
        entityMap_ = new ArsLexis::Hashtable();

    entityMap_->put(entity, value);
}
