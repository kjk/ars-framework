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

        // static const char_t * UNEXPECTED_EOF = "Unexpected EOF";
        // static const char_t * ILLEGAL_TYPE = "Wrong event type";
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
    
        String encoding;
        char_t *srcBuf;
    
        int srcPos;
        int srcCount;

        int line;
        int column;

        // txtbuffer
        char_t txtBuf[128];
        int txtPos;

        // Event-related
        int type;
        String text;
        bool isWhitespace;
        String nameSpace;
        String prefix;
        String name;

        bool degenerated;
        int attributeCount;
        String attributes[16];

        /** 
        * A separate peek buffer seems simpler than managing
        * wrap around in the first level read buffer */
        int peek[2];
        int peekCount;
        bool wasCR;
        bool unresolved;
        bool token;

    private:
        String  resolveEntity(String entity);

    public:
        KXmlParser();
        void    setInput(XmlReader *reader);
        void    setFeature(String feature, bool flag);
        void    nextToken();
        void    next();
        String  getPositionDescription();
        int     getEventType();
        void    defineEntityReplacementText(String entity, String value);
    };
}

#endif
