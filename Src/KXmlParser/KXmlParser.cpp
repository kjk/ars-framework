/*
  Implementation of XmlPull XML parsing API. It's a port of Java's kXML 2
  (http://kxml.org, http://kobjects.dyndns.org/kobjects/auto?self=%23c0a80001000000f5ad6a6fb3)
*/

#include "KxmlParser.hpp"

using namespace KXml2;

#define UNEXPECTED_EOF _T("Unexpected EOF")
#define ILLEGAL_TYPE _T("Wrong event type")


/* KXmlParser() */
KXmlParser::KXmlParser()
{
    fRelaxed_ = false;    
    //srcBufLenght_ = 128;
    //srcBuf_ = new (char_t[srcBufLenght_]);
    //txtBufSize_ = 128;
    //txtBuf_ = new (char_t[txtBufSize_]);
    
    srcBuf_ = String(128,' ');
    txtBuf_ = "";
}

/* PRIVATE */
int KXmlParser::peek(int pos) {
        while (pos >= peekCount_) {
            int nw;

            if (srcBuf_.length() <= 1)
                nw = reader_->read();
            else if (srcPos_ < srcCount_)
                nw = srcBuf_[srcPos_++];
            else {
                srcCount_ = reader_->read(&srcBuf_, 0, srcBuf_.length());
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
        return peek_[pos];
}

int KXmlParser::peekType(){
        switch (peek(0)) {
            case -1 :
                return END_DOCUMENT;
            case '&' :
                return ENTITY_REF;
            case '<' :
                switch (peek(1)) {
                    case '/' :
                        return END_TAG;
                    case '?' :
                    case '!' :
                        return LEGACY;
                    default :
                        return START_TAG;
                }
            default :
                return TEXT;
        }
}

int KXmlParser::read(){
        int result;

        if (peekCount_ == 0)
            result = peek(0);
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
        return result;
}


String KXmlParser::get(int pos) {
        return txtBuf_.substr (pos, txtPos_ - pos);
}

void KXmlParser::push(int c) {

        isWhitespace_ &= c <= ' ';

        if (txtPos_ == txtBuf_.length()) {
            //char_t *bigger = new char_t[(txtBufSize_ * 1.5)];
            //memmove(bigger,txtBuf_,txtBufSize_);
            //delete txtBuf_;
            //txtBuf_ = bigger;
            //txtBufSize_ *= 1.5;
            txtBuf_ += "                              ";
        }
        txtBuf_[txtPos_++] = (char) c;
}

void KXmlParser::skip() {
        while (true) {
            int c = peek(0);
            if (c > ' ' || c == -1)
                break;
            read();
        }
}



void KXmlParser::pushEntity() {

        read(); // &

        int pos = txtPos_;

        while (true) {
            int c = read();
            if (c == ';')
                break;
            if (c < 128 
                && (c < '0' || c > '9')
                && (c < 'a' || c > 'z')
                && (c < 'A' || c > 'Z')
                && c != '_' && c != '-' && c != '#') {
                    if(!relaxed_){
                        exception("unterminated entity ref"); //; ends with:"+(char)c);            
                    }
                    if (c != -1) push(c);                
                    return;
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
            return;
        }

        String result = (String) entityMap_->get(code);

        unresolved_ = result == "";

        if (unresolved_) {
            //kjk changes
            //if (!token)
            if (!token_ && !relaxed_)
            {
                exception((char_t *)&(((String)("unresolved: &" + code + ";"))[0]));
            }
        }
        else {
            for (int i = 0; i < result.length(); i++)
                push(result[i]);
        }
}

String KXmlParser::readName() {

        int pos = txtPos_;
        int c = peek(0);
        if ((c < 'a' || c > 'z')
            && (c < 'A' || c > 'Z')
            && c != '_'
            && c != ':'
            && c < 0x0c0)
            exception("name expected");
        do {
            push(read());
            c = peek(0);
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
        return result;
}

void KXmlParser::parseStartTag(bool xmldecl) {
/*
        if (!xmldecl)
            read();
        name_ = readName();
        attributeCount_ = 0;

        while (true) {
            skip();

            int c = peek(0);

            if (xmldecl) {
                if (c == '?') {
                    read();
                    read('>');
                    return;
                }
            }
            else {
                if (c == '/') {
                    degenerated_ = true;
                    read();
                    skip();
                    read('>');
                    break;
                }

                if (c == '>' && !xmldecl) {
                    read();
                    break;
                }
            }

            if (c == -1)
                exception(UNEXPECTED_EOF);

            String attrName = readName();

            if (attrName.length() == 0)
                exception("attr name expected");

            // kjk modifications
            int tmpC = peek(0);
            boolean fCanBeAttrWithNoValue=false;
            if (tmpC==' ' || tmpC=='>')
                fCanBeAttrWithNoValue=true;
            skip();
            tmpC = peek(0);
            if (tmpC!='=' && relaxed && fCanBeAttrWithNoValue)
            {
                int i = (attributeCount++) << 2;
    
                attributes = ensureCapacity(attributes, i + 4);
    
                attributes[i++] = "";
                attributes[i++] = null;
                attributes[i++] = attrName;
                attributes[i] = "";
            }
            else
            {
                read('=');
                skip();
                int delimiter = read();
    
                if (delimiter != '\'' && delimiter != '"') {
                    if (!relaxed)
                        exception(
                            "<"
                                + name
                                + ">: invalid delimiter: "
                                + (char) delimiter);
    
                    delimiter = ' ';
                }
    
                int i = (attributeCount++) << 2;
    
                attributes = ensureCapacity(attributes, i + 4);
    
                attributes[i++] = "";
                attributes[i++] = null;
                attributes[i++] = attrName;
    
                int p = txtPos;
                pushText(delimiter, true);
    
                attributes[i] = get(p);
                txtPos = p;
    
                if (delimiter != ' ')
                    read(); // skip endquote
            }
        }

        int sp = depth++ << 2;

        elementStack = ensureCapacity(elementStack, sp + 4);
        elementStack[sp + 3] = name;

        if (depth >= nspCounts.length) {
            int[] bigger = new int[depth + 4];
            System.arraycopy(nspCounts, 0, bigger, 0, nspCounts.length);
            nspCounts = bigger;
        }

        nspCounts[depth] = nspCounts[depth - 1];

        if (!relaxed)
        {
            for (int i = attributeCount - 1; i > 0; i--) {
                for (int j = 0; j < i; j++) {
                    if (getAttributeName(i).equals(getAttributeName(j)))
                        exception("Duplicate Attribute: " + getAttributeName(i));
                }
            }
        }

        if (processNsp)
            adjustNsp();
        else
            namespace = "";

        elementStack[sp] = namespace;
        elementStack[sp + 1] = prefix;
        elementStack[sp + 2] = name;

*/
}

/** 
* common base for next and nextToken. Clears the state, except from 
* txtPos and whitespace. Does not set the type variable */
void KXmlParser::nextImpl(){

        if (reader_ == NULL)
            exception("No Input specified");

        if (type_ == END_TAG)
            depth_--;

        while(true) {
            attributeCount_ = -1;

            if (degenerated_) {
                degenerated_ = false;
                type_ = END_TAG;
                return;
            }

            prefix_ = "";
            name_ = "";
            nameSpace_ = "";
            text_ = "";

            type_ = peekType();

            switch (type_) {

                case ENTITY_REF :
                    pushEntity();
                    return;

                case START_TAG :
                    parseStartTag(false);
                    return;
/*
                case END_TAG :
                    parseEndTag();
                    return;

                case END_DOCUMENT :
                    return;

                case TEXT :
                    pushText('<', !token_);
                    if (depth_ == 0) {
                        if (isWhitespace_)
                            type_ = IGNORABLE_WHITESPACE;
                        // make exception switchable for instances.chg... !!!!
                        //	else 
                        //    exception ("text '"+getText ()+"' not allowed outside root element");
                    }
                    return;

                default :
                    type_ = parseLegacy(token_);
                    if (type_ != XML_DECL)
                        return;
                        */
            }
        }
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

int  KXmlParser::next()
{
        txtPos_ = 0;
        isWhitespace_ = true;
        token_ = false;
        int minType = 9999;

        do {
            nextImpl();
            if (type_ < minType)
                minType = type_;
            //	    if (curr <= TEXT) type = curr;  //was commented in java
        }
        while (minType > ENTITY_REF // ignorable
            || (minType >= TEXT && peekType() >= TEXT));

        type_ = minType;
        if (type_ > TEXT)
            type_ = TEXT;

        return type_;
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

