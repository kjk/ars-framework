#ifndef __KXML2_KXML_PARSER_HPP__
#define __KXML2_KXML_PARSER_HPP__

#include <vector>
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
        std::vector<String>  elementStack_;
        std::vector<String>  nspStack_;
        std::vector<int>     nspCounts_;
          
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
        std::vector<String> attributes_;

        /** 
        * A separate peek buffer seems simpler than managing
        * wrap around in the first level read buffer */
        int peek_[2];
        int peekCount_;
        bool wasCR_;
        bool unresolved_;
        bool token_;

    private:
        void    ensureCapacity(std::vector<String>& vect, int size);
        void    ensureCapacityInt(std::vector<int>& vect, int size);
        String  resolveEntity(String entity);
        error_t peek(int& ret, int pos);
        error_t peekType(int& ret);
        error_t nextImpl();
        error_t pushEntity();
        error_t pushText(int delimiter, bool resolveEntities);
        String  get(int pos);
        void    push(int c);
        error_t read(const char c);
        error_t read(int& ret);
        error_t readName(String& ret);
        error_t skip();
        error_t parseStartTag(bool xmldecl);
        error_t adjustNsp(bool& ret);
    public:
        KXmlParser();
        void    setInput(XmlReader *reader);
        void    setFeature(String feature, bool flag);
        void    nextToken();
        error_t next(int& ret);
        String  getPositionDescription();
        int     getEventType();
        void    defineEntityReplacementText(String entity, String value);

    private: //public in java!
        error_t getAttributeName(String& ret, int index);
        String  getNamespace(String prefix);
        error_t getNamespaceCount(int& ret, int depth);
    };
}

#endif
