/*
  Implementation of XmlPull XML parsing API. It's a port of Java's kXML 2
  (http://kxml.org, http://kobjects.dyndns.org/kobjects/auto?self=%23c0a80001000000f5ad6a6fb3)
*/

#include "KXmlParser.hpp"
#include <Text.hpp>

using namespace std;
using namespace KXml2;

#ifdef __MWERKS__
# pragma pcrelconstdata on
# pragma far_code
# pragma inline_bottom_up on 
//# pragma inline_depth(100)
#endif

#define MUSTDO

//from XmlPullParser
#define NO_NAMESPACE _T("")

/*Init TYPES (we don't have XmlPullParser.cpp)*/
const XmlPullParser::TypeDescription_t XmlPullParser::TYPES[XmlPullParser::typesArrayLength] = 
{
    _T("START_DOCUMENT"),
    _T("END_DOCUMENT"),
    _T("START_TAG"),
    _T("END_TAG"),
    _T("TEXT"),
    _T("CDSECT"),
    _T("ENTITY_REF"),
    _T("IGNORABLE_WHITESPACE"),
    _T("PROCESSING_INSTRUCTION"),
    _T("COMMENT"),
    _T("DOCDECL")
};

const char_t* XmlPullParser::FEATURE_PROCESS_NAMESPACES = _T("http://xmlpull.org/v1/doc/features.html#process-namespaces");
const char_t* XmlPullParser::FEATURE_COMPLETLY_LOOSE = _T("http://dev.arslexis.com/w/index.php/KXmlCompletlyLoose");

inline XmlPullParser::EventType KXmlParser::getEventType()
{
    return type_;
}

KXmlParser::KXmlParser():
    reader_(0)
{
    fRelaxed_ = false;   
    completlyLoose_ = false;
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
            if ((error=reader_->read(nw)) != eNoError)
                return error;
        }       
        else if (srcPos_ < srcCount_)
            nw = srcBuf_[srcPos_++];
        else {
            if ((error=reader_->read(srcCount_, srcBuf_, 0, srcBuf_.length())) != eNoError)
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

error_t KXmlParser::peekType(EventType& ret)
{
    error_t error;
    int     retChild;

    if ((error=peek(retChild,0))!=eNoError)
        return error;

    switch (retChild) 
    {
        case -1 :
            ret = END_DOCUMENT;
            return eNoError;
        case _T('&') :
            ret = ENTITY_REF;
            return eNoError;
        case _T('<') :
            if ((error=peek(retChild,1))!=eNoError)
                return error;
            switch (retChild) 
            {
                case _T('/') :
                    ret = END_TAG;
                    return eNoError;
                case _T('?') :
                case _T('!') :
                    ret = static_cast<EventType>(LEGACY);
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
    isWhitespace_ &= (c <= _T(' '));

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
        if (c > _T(' ') || c == -1)
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
    if (c == _T(' '))
    {
        push(_T('&'));
        return eNoError;
    }

    int pos = txtPos_;

    while (true) 
    {
        if ((error=read(c))!=eNoError) 
            return error;

        if (c == _T(';'))
            break;

        if (c < 128 
            && (c < _T('0') || c > _T('9'))
            && (c < _T('a') || c > _T('z'))
            && (c < _T('A') || c > _T('Z'))
            && c != _T('_') && c != _T('-') && c != _T('#'))
        {
                if (!relaxed_)
                    return eUnterminatedEntityRef;
                if (c != -1) 
                    push(c);
                /*
                TODO: Think about sth like this:
                text: "Starsky &Hatch blah blah"
                we push "Starsky Hatch bla bla"
                we dont push & if its not before space "& " -> push(_T('&'))
                if after & is text "&blah blah" we dont put "&blah" -> "blah"
                if we want to put "&blah" -> "&blah" we should do this:
                
                String code = get(pos); //get all what we have pushed so far
                txtPos_ = pos;
                push(_T('&'));              //push & before it
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

    if (_T('#')==code[0])
     {
        const char_t* begin=code.data()+1;
        const char_t* end=begin+code.length();
        long c;
        error_t err;
        if (_T('x')==*begin)
            err=ArsLexis::numericValue(++begin, end, c, 16);
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
    if ((error=peek(c,0))!=eNoError)
        return error;

    if ((c < _T('a') || c > _T('z'))
        && (c < _T('A') || c > _T('Z'))
        && c != _T('_')
        && c != _T(':')
        && c < 0x0c0)
        return eNameExpected;

    do
    {
        if ((error=read(c))!=eNoError)
            return error;
        push(c);
        if ((error=peek(c,0))!=eNoError)
            return error;
    } while ((c >= _T('a') && c <= _T('z'))
        || (c >= _T('A') && c <= _T('Z'))
        || (c >= _T('0') && c <= _T('9'))
        || c == _T('_')
        || c == _T('-')
        || c == _T(':')
        || c == _T('.')
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
    { // covers eof, _T('<'), _T('"')

        if (delimiter == _T(' '))
            if (next <= _T(' ') || next == _T('>'))
                break;

        if (next == _T('&'))
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
            push(_T(' '));
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
    if (0==prefix.compare(_T("xml")))
        return _T("http://www.w3.org/XML/1998/namespace");
    if (0==prefix.compare(_T("xmlns")))
        return _T("http://www.w3.org/2000/xmlns/");

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
        String::size_type cut = attrName.find(_T(':'));
        String prefix;

        if (cut != String::npos)
        {
            prefix.assign(attrName, 0, cut);
            attrName.erase(0, cut + 1);
        }
        else if (0==attrName.compare(_T("xmlns")))
        {
            prefix_.assign(attrName);
            attrName=_T("");
        }
        else
            continue;

        if (0!=prefix_.compare(_T("xmlns")))
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
            String::size_type cut = attrName.find(_T(':'));

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

    String::size_type cut = name_.find(_T(':'));
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
            if (c == _T('?')) {
                if ((error=read(c))!=eNoError)
                    return error;
                if ((error=read(_T('>')))!=eNoError)
                    return error;

                return eNoError;
            }
        }
        else {
            if (c == _T('/')) {
                degenerated_ = true;
                if ((error=read(tmp))!=eNoError)
                    return error;
                if ((error=skip())!=eNoError)
                    return error;
                if ((error=read(_T('>')))!=eNoError)
                    return error;
                break;
            }

            if (c == _T('>') && !xmldecl) {
                if ((error=read(tmp))!=eNoError)
                    return error;
                break;
            }
        }

        if (c == -1)
            return eUnexpectedEof;

        String attrName;
        int tmpC;
        if ((error=peek(tmpC,0))!=eNoError)
            return error;
        
        if(tmpC == '\"' && completlyLoose_)
        {
            attrName = _T(" "); //no name, but must be not empty (happen in <AD- "http://...."> tegs)
        }
        else
            if ((error=readName(attrName))!=eNoError)
                return error;
        
        if (attrName.empty())
            return eAttrNameExpected;

        if ((error=peek(tmpC,0))!=eNoError)
            return error;
        bool fCanBeAttrWithNoValue=false;
        if (tmpC==_T(' ') || tmpC==_T('>'))
            fCanBeAttrWithNoValue=true;
        if ((error=skip())!=eNoError)
            return error;
        if ((error=peek(tmpC,0))!=eNoError)
            return error;
        if (tmpC!=_T('=') && relaxed_ && fCanBeAttrWithNoValue)
        {
            int i = (attributeCount_++) << 2;
            attributes_.resize(i + 4);
            i+=2;
            attributes_[i++] = attrName;
        }
        else
        {
            if(tmpC == '\"' && completlyLoose_)
            {
                //nothing (dont read '=' becouse there is no '=' )
            }
            else
                if ((error=read(_T('=')))!=eNoError)
                    return error;

            if ((error=skip())!=eNoError)
                return error;
            int delimiter;
            if ((error=read(delimiter))!=eNoError)
                return error;

            int p = txtPos_;

            if (delimiter != _T('\'') && delimiter != _T('"')) {
                if (!relaxed_)
                {
                    return eInvalidDelimiter;
                }
                /* to detect attributes without "
                   for example: bgcolor=eeaaaa 
                   delimiter = _T('e')
                */
                if (delimiter != _T(' '))
                    push(delimiter);

                delimiter = _T(' ');
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

            if (delimiter != _T(' '))
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
        
    if ((error=read(temp))!=eNoError) // _T('<')
        return error;
    if ((error=read(temp))!=eNoError) // _T('/')
        return error;

    if ((error=readName(name_))!=eNoError) 
        return error;
    if ((error=skip())!=eNoError) 
        return error;
    if ((error=read(_T('>')))!=eNoError) 
        return error;

    int sp = (depth_ - 1) << 2;

    if (!relaxed_) {

        if (depth_ == 0)
            return eElementStackEmpty;

        if (name_!=elementStack_[sp + 3])
            return eExpectedDifferentName;
    }
    else
        if (depth_ == 0 || !ArsLexis::equalsIgnoreCase(name_, elementStack_[sp + 3]))
            return eNoError; //was just return, so no error
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

            case _T('\'') :
                quoted = !quoted;
                break;

            case _T('<') :
                if (!quoted)
                    nesting++;
                break;

            case _T('>') :
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

error_t KXmlParser::parseLegacy(EventType& ret, bool pushV) {
    error_t error;
    String req;
    int term;
    EventType result;
    int prev = 0;
    int c;
    int tmp1,tmp2;

    if ((error=read(c))!=eNoError) // <
        return error;
    if ((error=read(c))!=eNoError)
        return error;

    if (c == _T('?')) {
        if ((error=peek(tmp1,0))!=eNoError)
            return error;
        if ((error=peek(tmp2,1))!=eNoError)
            return error;

        if ((tmp1 == _T('x') || tmp1 == _T('X'))
            && (tmp2 == _T('m') || tmp2 == _T('M'))) {

            if (pushV) {
                push(tmp1);
                push(tmp2);
            }
            if ((error=read(tmp1))!=eNoError)
                return error;
            if ((error=read(tmp1))!=eNoError)
                return error;


            if ((error=peek(tmp1,0))!=eNoError)
                return error;
            if ((error=peek(tmp2,1))!=eNoError)
                return error;
            if ((tmp1 == _T('l') || tmp1 == _T('L')) && tmp2 <= _T(' ')) {

                if (line_ != 1 || column_ > 4)
                    return ePIMustNotStartWithXml;

                if ((error=parseStartTag(true))!=eNoError)
                    return error;

                if (attributeCount_ < 1 || 0!=attributes_[2].compare(_T("version")))
                    return eVersionExpected;

                version_.assign(attributes_[3]);

                int pos = 1;

                if (pos < attributeCount_ && 0==attributes_[2+4].compare(_T("encoding")))  {
                    encoding_.assign(attributes_[3+4]);
                    pos++;                        
                }

                if (pos < attributeCount_ && 0==attributes_[4*pos+2].compare(_T("standalone"))) {
                    const String& st = attributes_[3+4*pos];
                    if (0==st.compare(_T("yes"))) 
                        standalone_ = true;
                    else if (0==st.compare(_T("no"))) 
                        standalone_ = false;
                    else 
                        return eIllegalStandaloneValue;
                    pos++;
                }

                if (pos != attributeCount_) 
                    return eIllegalXmldecl;

                isWhitespace_ = true;
                txtPos_ = 0;                    

                ret = static_cast<EventType>(XML_DECL);
                return eNoError;
            }
        }

        term = _T('?');
        result = PROCESSING_INSTRUCTION;
    }
    else if (c == _T('!')) {
        if ((error=peek(tmp1,0))!=eNoError)
            return error;
        if (tmp1 == _T('-')) {
            result = COMMENT;
            req = _T("--");
            term = _T('-');
        }
        else if (tmp1 == _T('[')) {
            result = CDSECT;
            req = _T("[CDATA[");
            term = _T(']');
            pushV = true;
        }
        else {
            result = DOCDECL;
            req = _T("DOCTYPE");
            term = -1;
        }
    }
    else 
        return eIllegalStartTag;

    for (size_t i = 0; i < req.length(); i++)
        if ((error=read(req[i]))!=eNoError)
            return error;

    
    if (result == DOCDECL)
    {
        if ((error=parseDoctype(pushV))!=eNoError)
            return error;
    }
    else {
        while (true) {
            if ((error=read(c))!=eNoError)
                return error;
            if (c == -1)
                return eUnexpectedEof;

            if (pushV)
                push(c);

            if ((error=peek(tmp1,0))!=eNoError)
                return error;
            if ((error=peek(tmp2,1))!=eNoError)
                return error;

            if ((term == _T('?') || c == term)
                && tmp1 == term
                && tmp2 == _T('>'))
                break;

            prev = c;
        }

        if (term == _T('-') && prev == _T('-') && !relaxed_)
            return eIllegalCommentDelimiter;

        if ((error=read(tmp1))!=eNoError)
            return error;
        if ((error=read(tmp1))!=eNoError)
            return error;

        if (pushV && term != _T('?'))
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

        if ((error=peekType(type_))!=eNoError)
            return error;

        switch (type_)
        {
            case ENTITY_REF :
                if ((error=pushEntity())!=eNoError)
                    return error;
                return eNoError;

            case START_TAG :
                if ((error=parseStartTag(false))!=eNoError)
                    return error;
                return eNoError;

            case END_TAG :
                if ((error=parseEndTag())!=eNoError)
                    return error;
                return eNoError;

            case END_DOCUMENT :
                return eNoError;

            case TEXT :
                if ((error=pushText(_T('<'), !token_))!=eNoError)
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
                if ((error=parseLegacy(type_,token_))!=eNoError)
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
        if (n1.compare(0, 26, _T("http://xmlpull.org/v1/doc/"))!=0) return false;
        if (prop) 
            return 0==n1.compare(42, String::npos, n2);
        else 
            return 0==n1.compare(40, String::npos, n2);
}

error_t KXmlParser::setFeature(const String& feature, bool flag)
{
    if (0==feature.compare(FEATURE_PROCESS_NAMESPACES))
        processNsp_ = flag;
    else
        if (isProp(feature, false, _T("relaxed")))
            relaxed_ = flag;
        else
            if (0==feature.compare(FEATURE_COMPLETLY_LOOSE))
                completlyLoose_ = flag;
            else
                return eUnsupportedFeature;

    return eNoError;
}

error_t KXmlParser::nextToken(EventType& ret)
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

error_t KXmlParser::next(EventType& ret)
{
    error_t error;
    EventType tempT;
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

    type_ = static_cast<EventType>(minType);
    if (type_ > TEXT)
        type_ = TEXT;

    ret = type_;
    return eNoError;
}

bool KXmlParser::getFeature(const String& feature) 
{
    if (0==feature.compare(FEATURE_PROCESS_NAMESPACES))
        return processNsp_;
    else 
        if (isProp(feature, false, _T("relaxed")))
            return relaxed_;
        else            
            if(0==feature.compare(FEATURE_COMPLETLY_LOOSE))
                return completlyLoose_;
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
        if (0==attributes_[i + 2].compare(name) && (nameSpace.empty() || 0==attributes_[i].compare(nameSpace)))
            return attributes_[i + 3];
    return String();
}

error_t KXmlParser::nextTag(EventType& ret) 
{
    error_t error;
    EventType i;
    
    if ((error=next(i))!=eNoError)
        return error;
    if (type_ == TEXT && isWhitespace_)
        if ((error=next(i))!=eNoError)
            return error;

    if (type_ != END_TAG && type_ != START_TAG)
        return eUnexpectedType;

    ret = type_;
    return eNoError;
}

error_t KXmlParser::nextText(String& ret)
{
    error_t error;
    EventType i;
    if (type_ != START_TAG)
        return ePreconditionStartTag;

    if ((error=next(i))!=eNoError)
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
        buf = _T("unknown");

    buf += _T(' ');

    if (type_ == START_TAG || type_ == END_TAG)
    {
        if (degenerated_)
            buf += _T("(empty) ");
        buf += _T('<');
        if (type_ == END_TAG)
            buf += _T('/');

        if (!prefix_.empty())
            buf.append(1, _T('{')).append(nameSpace_).append(1, _T('}')).append(prefix_).append(1, _T(':'));
        buf += name_;

        int cnt = attributeCount_ << 2;
        for (int i = 0; i < cnt; i += 4)
        {
            buf.append(1, _T(' '));
            if (!attributes_[i + 1].empty())
                buf.append(1, _T('{')).append(attributes_[i]).append(1, _T('}')).append(attributes_[i + 1]).append(1, _T(':'));
            buf.append(attributes_[i + 2]).append(_T("='")).append(attributes_[i + 3]).append(1, _T('\''));
        }
        buf.append(1, _T('>'));
    }
    else if (type_ == IGNORABLE_WHITESPACE)
    {
        // do nothing
    }
    else if (type_ != TEXT)
    {
        String text = getText();
        if ( ((ENTITY_REF == type_) || (END_DOCUMENT == type_)) && (text.empty()))
            text = _T("null");

       buf += text;
    }
    else if (isWhitespace_)
        buf += _T("(whitespace)");
    else
    {
        String text = getText();
        if (text.length() > 16)
        {
            text.resize(16);
            text.append(_T("..."));
        }            
        buf.append(text);            
    }

    buf.append(_T(" @"));
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
    if (0==entity.compare(_T("amp"))) 
        return _T("&");
    if (0==entity.compare(_T("apos"))) 
        return _T("'");
    if (0==entity.compare(_T("gt"))) 
        return _T(">");
    if (0==entity.compare(_T("lt"))) 
        return _T("<");
    if (0==entity.compare(_T("quot"))) 
        return _T("\"");

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
