/*
  Implementation of XmlPull XML parsing API. It's a port of Java's kXML 2
  (http://kxml.org, http://kobjects.dyndns.org/kobjects/auto?self=%23c0a80001000000f5ad6a6fb3)
*/

#include "KxmlParser.hpp"

using namespace std;
using namespace KXml2;

// private static const...
#define UNEXPECTED_EOF _T("Unexpected EOF")
#define ILLEGAL_TYPE _T("Wrong event type")

//from XmlPullParser
#define NO_NAMESPACE ""

/* KXmlParser() */
KXmlParser::KXmlParser()
{
    fRelaxed_ = false;    
    
    srcBuf_ = String(128,' ');
    txtBuf_ = String(128,' ');

    ensureCapacity(attributes_, 16);
    ensureCapacity(elementStack_, 16);
    ensureCapacity(nspStack_, 8);
    ensureCapacityInt(nspCounts_, 4);
}

/* PRIVATE */
void KXmlParser::ensureCapacity(vector<String>& vect, int size)
{
    vect.resize(size);
}

void KXmlParser::ensureCapacityInt(vector<int>& vect, int size)
{
    vect.resize(size);
}

error_t KXmlParser::peek(int& ret, int pos) {
    error_t Error;
        while (pos >= peekCount_) {
            int nw;

            if (srcBuf_.length() <= 1)
            {
                if((Error=reader_->read(nw)) != eNoError)
                    return Error;
            }       
            else if (srcPos_ < srcCount_)
                nw = srcBuf_[srcPos_++];
            else {
                if((Error=reader_->read(srcCount_, srcBuf_, 0, srcBuf_.length())) != eNoError)
                    return Error;
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

error_t KXmlParser::peekType(int& ret){
    error_t Error;
    int retChild;
    if((Error=peek(retChild,0))!=eNoError)
        return Error;
        switch (retChild) {
            case -1 :
                ret = END_DOCUMENT;
                return eNoError;
            case '&' :
                ret = ENTITY_REF;
                return eNoError;
            case '<' :
                if((Error=peek(retChild,1))!=eNoError)
                    return Error;
                switch (retChild) {
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

error_t KXmlParser::read(const char c) {
    error_t Error;    
        int a;
        if((Error=read(a))!=eNoError)
            return Error;
        if (a != c)
            return eExpectedCharNotEqual;
    return eNoError;
}

error_t KXmlParser::read(int& ret){
    error_t Error;        
    int result;

        if (peekCount_ == 0)
        {
            if((Error=peek(result,0))!=eNoError)
                return Error;
        }
        else {
            result = peek_[0];
            peek_[0] = peek_[1];
        }
        peekCount_--;

        column_++;

        if (result == '\n') {
            line_++;
            column_ = 1;
        }
        
    ret = result;
    return eNoError;
}


String KXmlParser::get(int pos) {
        return txtBuf_.substr (pos, txtPos_ - pos);
}

void KXmlParser::push(int c) {

        isWhitespace_ &= c <= ' ';

        if (txtPos_ == txtBuf_.length()) {
            //TODO: stringLen = 4/3 * stringLen + 4;
            txtBuf_ += "                              ";
        }
        txtBuf_[txtPos_++] = (char) c;
}

error_t KXmlParser::skip() {
    error_t Error;    
    int c;
        while (true) {
            if((Error=peek(c,0))!=eNoError)
                return Error;
            if (c > ' ' || c == -1)
                break;
            if((Error=read(c))!=eNoError)
                return Error;
        }
    return eNoError;
}



error_t KXmlParser::pushEntity() {
    error_t Error;
        int   c;

        if((Error=read(c))!=eNoError) // &
            return Error;

        int pos = txtPos_;

        while (true) {
            if((Error=read(c))!=eNoError) 
                return Error;

            if (c == ';')
                break;
            if (c < 128 
                && (c < '0' || c > '9')
                && (c < 'a' || c > 'z')
                && (c < 'A' || c > 'Z')
                && c != '_' && c != '-' && c != '#') {
                    if(!relaxed_){
                        return eUnterminatedEntityRef;
                    }
                    if (c != -1) push(c);                
                    return eNoError;
                }

            push(c);
        }

        String code = get(pos);
        txtPos_ = pos;
        if (token_ && type_ == ENTITY_REF)
            name_ = code;

        if (code[0] == '#') {
            char_t *endPtr;

            int c =
                (code[1] == 'x'
                    ? (int) strtol(&code[2],&endPtr, 16)
                    : atoi(&code[1]));
            push(c);
            return eNoError;
        }

        String result = (String) entityMap_->get(code);

        unresolved_ = result == "";

        if (unresolved_) {
            //kjk changes
            //if (!token)
            if (!token_ && !relaxed_)
                return eUnresolvedCode;
        }
        else {
            for (int i = 0; i < result.length(); i++)
                push(result[i]);
        }
    return eNoError;
}

error_t KXmlParser::readName(String& ret) {
    error_t Error;
        int pos = txtPos_;
        int c;
        if((Error=peek(c,0))!=eNoError)
            return Error;

        if ((c < 'a' || c > 'z')
            && (c < 'A' || c > 'Z')
            && c != '_'
            && c != ':'
            && c < 0x0c0)
            return eNameExpected;
        do {
            if((Error=read(c))!=eNoError)
                return Error;
            push(c);
            if((Error=peek(c,0))!=eNoError)
                return Error;
        }
        while ((c >= 'a' && c <= 'z')
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

error_t KXmlParser::pushText(int delimiter, bool resolveEntities) {
    error_t Error;
        int next;
        if((Error=peek(next,0))!=eNoError)
            return Error;

        while (next != -1 && next != delimiter) { // covers eof, '<', '"'

            if (delimiter == ' ')
                if (next <= ' ' || next == '>')
                    break;

            if (next == '&') {
                if (!resolveEntities)
                    break;
                
                if((Error=pushEntity())!=eNoError)
                    return Error;
            }
            else if (next == '\n' && type_==START_TAG) {
                if((Error=read(next))!=eNoError)
                    return Error;
                push(' ');
            }
            else
            {
                if((Error=read(next))!=eNoError)
                    return Error;
                push(next);
            }
            
            if((Error=peek(next,0))!=eNoError)
                return Error;
        }
    return eNoError;
}

error_t KXmlParser::getNamespaceCount(int& ret, int depth) {
    error_t Error;
    if (depth > depth_)
            return eIndexOutOfBoundsException;
    ret = nspCounts_[depth];
    return eNoError; 
}


String KXmlParser::getNamespace(String prefix) {

        if ("xml" == prefix)
            return "http://www.w3.org/XML/1998/namespace";
        if ("xmlns" == prefix)
            return "http://www.w3.org/2000/xmlns/";

        int j;
        getNamespaceCount(j, depth_);  //It's 100% safe: depth_ <= depth_
        for (int i = (j << 1) - 2; i >= 0; i -= 2) {
            if (prefix == "") {
                if (nspStack_[i] == "")
                    return nspStack_[i + 1];
            }
            else if (prefix == nspStack_[i])
                return nspStack_[i + 1];
        }
        return "";
}

error_t KXmlParser::adjustNsp(bool& ret) {
        bool any = false;

        for (int i = 0; i < attributeCount_ << 2; i += 4) {

            String attrName = attributes_[i + 2];
            int cut = attrName.find(':');
            String prefix;

            if (cut != -1) {
                prefix = attrName.substr(0, cut);
                attrName = attrName.substr(cut + 1);
            }
            else if (attrName == "xmlns") {
                prefix_ = attrName;
                attrName = "";
            }
            else
                continue;

            if (prefix_ != "xmlns" ) {
                any = true;
            }
            else {
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

        if (any) {
            for (int i = (attributeCount_ << 2) - 4; i >= 0; i -= 4) {

                String attrName = attributes_[i + 2];
                int cut = attrName.find(':');

                if (cut == 0 && !relaxed_)
                    return eIllegalAttributeName;
                else if (cut != -1) {
                    String attrPrefix = attrName.substr(0, cut);

                    attrName = attrName.substr(cut + 1);

                    String attrNs = getNamespace(attrPrefix);

                    if (attrNs == "" && !relaxed_)
                        return eUndefinedPrefix;

                    attributes_[i] = attrNs;
                    attributes_[i + 1] = attrPrefix;
                    attributes_[i + 2] = attrName;

                    if (!relaxed_) {
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
        else if (cut != -1) {
            prefix_ = name_.substr(0, cut);
            name_ = name_.substr(cut + 1);
        }

        nameSpace_ = getNamespace(prefix_);

        if (nameSpace_ == "") {
            if (prefix_ != "" && !relaxed_)
                return eUndefinedPrefix;
            nameSpace_ = NO_NAMESPACE;
        }

        ret = any;
    return eNoError;
}

error_t KXmlParser::parseStartTag(bool xmldecl) {
    error_t Error;
        int c,tmp;
        if (!xmldecl)
            if((Error=read(c))!=eNoError)
                return Error;

        if((Error=readName(name_))!=eNoError)
            return Error;

        attributeCount_ = 0;

        while (true) {
            if((Error=skip())!=eNoError)
                return Error;

            if((Error=peek(c,0))!=eNoError)
                return Error;

            if (xmldecl) {
                if (c == '?') {
                    if((Error=read(c))!=eNoError)
                        return Error;
                    if((Error=read('>'))!=eNoError)
                        return Error;

                    return eNoError;
                }
            }
            else {
                if (c == '/') {
                    degenerated_ = true;
                    if((Error=read(tmp))!=eNoError)
                        return Error;
                    if((Error=skip())!=eNoError)
                        return Error;
                    if((Error=read('>'))!=eNoError)
                        return Error;
                    break;
                }

                if (c == '>' && !xmldecl) {
                    if((Error=read(tmp))!=eNoError)
                        return Error;
                    break;
                }
            }

            if (c == -1)
                return eUnexpectedEof;

            String attrName;
            if((Error=readName(attrName))!=eNoError)
                return Error;

            if (attrName.length() == 0)
                return eAttrNameExpected;

            // kjk modifications
            int tmpC;
            if((Error=peek(tmpC,0))!=eNoError)
                return Error;
            bool fCanBeAttrWithNoValue=false;
            if (tmpC==' ' || tmpC=='>')
                fCanBeAttrWithNoValue=true;
            if((Error=skip())!=eNoError)
                return Error;
            if((Error=peek(tmpC,0))!=eNoError)
                return Error;
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
                if((Error=read('='))!=eNoError)
                    return Error;
                if((Error=skip())!=eNoError)
                    return Error;
                int delimiter;
                if((Error=read(delimiter))!=eNoError)
                    return Error;
    
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
                if((Error=pushText(delimiter, true))!=eNoError)
                    return Error;

                attributes_[i] = get(p);
                txtPos_ = p;
    
                if (delimiter != ' ')
                    if((Error=read(tmp))!=eNoError)// skip endquote
                        return Error;
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
                    if((Error=getAttributeName(attrTemp1,i))!=eNoError)
                        return Error;
                    if((Error=getAttributeName(attrTemp2,j))!=eNoError)
                        return Error;
                    if (attrTemp1 == attrTemp2)
                        return eDuplicateAttribute;
                }
            }
        }

        if (processNsp_)
        {
            bool tempB;
            if((Error=adjustNsp(tempB))!=eNoError)// skip endquote
                return Error;
        }
        else
            nameSpace_ = "";

        elementStack_[sp] = nameSpace_;
        elementStack_[sp + 1] = prefix_;
        elementStack_[sp + 2] = name_;

    return eNoError;
}

/** 
* common base for next and nextToken. Clears the state, except from 
* txtPos and whitespace. Does not set the type variable */
error_t KXmlParser::nextImpl(){
    error_t Error;

        if (reader_ == NULL)
            return eNoInputSpecified;

        if (type_ == END_TAG)
            depth_--;

        while(true) {
            attributeCount_ = -1;

            if (degenerated_) {
                degenerated_ = false;
                type_ = END_TAG;
                return eNoError;
            }

            prefix_ = "";
            name_ = "";
            nameSpace_ = "";
            text_ = "";

            if((Error=peekType(type_))!=eNoError);
                return Error;

            switch (type_) {

                case ENTITY_REF :
                    if((Error=pushEntity())!=eNoError)
                        return Error;
                    return eNoError;

                case START_TAG :
                    if((Error=parseStartTag(false))!=eNoError)
                        return Error;
                    return eNoError;
/*
                case END_TAG :
                    parseEndTag();
                    return eNoError;

                case END_DOCUMENT :
                    return eNoError;

                case TEXT :
                    pushText('<', !token_);
                    if (depth_ == 0) {
                        if (isWhitespace_)
                            type_ = IGNORABLE_WHITESPACE;
                        // make exception switchable for instances.chg... !!!!
                        //	else 
                        //    exception ("text '"+getText ()+"' not allowed outside root element");
                    }
                    return eNoError;

                default :
                    type_ = parseLegacy(token_);
                    if (type_ != XML_DECL)
                        return eNoError;
                        */
            }
        }
    //TODO: delete this when function complete
    return eNoError;
}




/* PUBLIC */

void KXmlParser::setInput(XmlReader *reader)
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
    standalone_ = "";

    if (reader_ == NULL)
        return;
 
    srcPos_ = 0;
    srcCount_ = 0;
    peekCount_ = 0;
    depth_ = 0;

    if (entityMap_)
    {
        entityMap_->Dispose();
        delete entityMap_;
        entityMap_ = NULL;
    }

}

void KXmlParser::setFeature(String feature, bool flag)
{
    if (feature==FEATURE_RELAXED)
    {
        fRelaxed_ = flag;
    }
    else
    {
        // TODO: throw an exception? do nothing?
    }
}

void KXmlParser::nextToken()
{
}

error_t KXmlParser::next(int& ret)
{
    error_t Error;
    int tempT;
        txtPos_ = 0;
        isWhitespace_ = true;
        token_ = false;
        int minType = 9999;

        do {
            nextImpl();
            if (type_ < minType)
                minType = type_;
            //	    if (curr <= TEXT) type = curr;  //was commented in java
            if((Error=peekType(tempT))!=eNoError)
                return Error;
        }
        while (minType > ENTITY_REF // ignorable
            || (minType >= TEXT && tempT >= TEXT));

        type_ = minType;
        if (type_ > TEXT)
            type_ = TEXT;

        ret = type_;
        return eNoError;
}

String KXmlParser::getPositionDescription()
{
    return "";
}

int KXmlParser::getEventType()
{
    return END_DOCUMENT;
}

// TODO: is this the right interface? What to do in case of not finding the substitution
String KXmlParser::resolveEntity(String entity)
{
    // first resolve standard entities
    // TODO: could be optimized further by doing case on first letter first
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
    if (NULL==entityMap_)
        entityMap_ = new ArsLexis::Hashtable();

    entityMap_->put(entity, value);
}
                               
error_t KXmlParser::getAttributeName(String& ret, int index) {
        if (index >= attributeCount_)
            //throw new IndexOutOfBoundsException();
            return eIndexOutOfBoundsException;
        ret = attributes_[(index << 2) + 2];
        return eNoError;
}
