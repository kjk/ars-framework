#ifndef __KXML2_KXML_PARSER_HPP__
#define __KXML2_KXML_PARSER_HPP__

#include "XmlPull.hpp"
#include <vector>
#include <map>
#include <memory>

namespace KXml2{

    #define FEATURE_RELAXED _T("http://xmlpull.org/v1/doc/features.html#relaxed")

    // Defines a concrete implementation of pull XML parsing
    class KXmlParser : public XmlPullParser
    {
    private:
        bool        fRelaxed_;
        Reader * reader_;

        static const int LEGACY = 999;
        static const int XML_DECL = 998;

        // general

        String          version_;
        bool            standalone_;

        bool            processNsp_;
        bool            relaxed_;
        typedef std::map<String, String> EntityMap_t;
        EntityMap_t entityMap_;
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
        String  resolveEntity(const String& entity);
        error_t peek(int& ret, int pos);
        error_t peekType(int& ret);
        error_t nextImpl();
        error_t pushEntity();
        error_t pushText(int delimiter, bool resolveEntities);
        String  get(int pos);
        void    push(int c);
        error_t read(const char_t c);
        error_t read(int& ret);
        error_t readName(String& ret);
        error_t skip();
        error_t parseStartTag(bool xmldecl);
        error_t adjustNsp(bool& ret);
        error_t parseEndTag();
        error_t parseLegacy(int& ret, bool pushV);
        error_t parseDoctype(bool pushV);
        bool    isProp (const String& n1, bool prop, const String& n2);
    public:
        KXmlParser();
        ~KXmlParser();
        String  getText();
        error_t setInput(Reader& reader);
        error_t setFeature(const String& feature, bool flag);
        error_t nextToken(int& ret);
        error_t next(int& ret);
        error_t getPositionDescription(String& ret);
        int     getEventType();
        void    defineEntityReplacementText(const String& entity, const String& value);

        bool    getFeature(const String& feature);
        String  getInputEncoding();
        String  getNamespacePrefix(int pos);
        String  getNamespaceUri(int pos);
        int     getDepth();
        int     getLineNumber();
        int     getColumnNumber();
        error_t isWhitespace(bool& ret);
        String  getNamespace();
        String  getName();
        String  getPrefix();
        error_t isEmptyElementTag(bool& ret);
        int     getAttributeCount();
        error_t getAttributeNamespace(String& ret, int index); 
        error_t getAttributeName(String& ret, int index);
        error_t getAttributePrefix(String& ret, int index);
        error_t getAttributeValue(String& ret, int index);
        String  getAttributeValue(const String& nameSpace, const String& name);
        String  getNamespace(const String& prefix);
        error_t getNamespaceCount(int& ret, int depth);
        error_t nextText(String& ret);
        error_t nextTag(int& ret);

    };
}

#endif
