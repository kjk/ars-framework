#ifndef __KXML2_KXML_PARSER_HPP__
#define __KXML2_KXML_PARSER_HPP__

#include "XmlPull.hpp"
#include "..\Hashtable.hpp"

namespace KXml2{
    #define FEATURE_RELAXED "http://xmlpull.org/v1/doc/features.html#relaxed"

    // Defines a concrete implementation of pull XML parsing
    class KXmlParser : public XmlPullParser
    {
    private:
        bool        fRelaxed_;
        XmlReader * reader_;

        //as #define in cpp.
        //static const char_t * UNEXPECTED_EOF = "Unexpected EOF";
        //static const char_t * ILLEGAL_TYPE = "Wrong event type";
        static const int LEGACY = 999;
        static const int XML_DECL = 998;

        // general

        String          version_;
        bool            standalone_;

        bool            processNsp_;
        bool            relaxed_;
        ArsLexis::Hashtable *entityMap_;
        int             depth_;
        String          elementStack_[16];
        String          nspStack_[8];
        int             nspCounts_[4];

        // source
    
        String encoding_;
        //char_t *srcBuf_;
        //int srcBufLenght_;
        String srcBuf_;
    
        int srcPos_;
        int srcCount_;

        int line_;
        int column_;

        // txtbuffer
        String txtBuf_;
        int txtPos_;

        // Event-related
        int type_;
        String text_;
        bool isWhitespace_;
        String nameSpace_;
        String prefix_;
        String name_;

        bool degenerated_;
        int attributeCount_;
        String attributes_[16];

        /** 
        * A separate peek buffer seems simpler than managing
        * wrap around in the first level read buffer */
        int peek_[2];
        int peekCount_;
        bool wasCR_;
        bool unresolved_;
        bool token_;

    private:
        String  resolveEntity(String entity);
        int     peek(int pos);
        int     peekType();
        void    nextImpl();
        void    pushEntity();
        String  get(int pos);
        void    push(int c);
        int     read();
        String  readName();
        void    skip();
        void    parseStartTag(bool xmldecl);

    public:
        KXmlParser();
        void    setInput(XmlReader *reader);
        void    setFeature(String feature, bool flag);
        void    nextToken();
        int     next();
        String  getPositionDescription();
        int     getEventType();
        void    defineEntityReplacementText(String entity, String value);
    };
}

#endif
