#ifndef __KXML2_XML_PULL_HPP__
#define __KXML2_XML_PULL_HPP__

#include "KXml2.hpp"
#include <Reader.hpp>
#include <vector>

// Defines a public, abstract interface for pull XML parsing
// This api is based on Java's XmlPull API (http://www.xmlpull.org/)
namespace KXml2{

    using ArsLexis::Reader;

    class XmlPullParser
    {
    public:

        enum EventType {
            START_DOCUMENT,
            END_DOCUMENT,
            START_TAG,
            END_TAG,
            TEXT,
            CDSECT,
            ENTITY_REF,
            IGNORABLE_WHITESPACE,
            PROCESSING_INSTRUCTION,
            COMMENT,
            DOCDECL,
        };
        
        typedef char_t TypeDescription_t[24];
        enum {typesArrayLength=11};
        static const TypeDescription_t TYPES[typesArrayLength];

        static const char_t* FEATURE_PROCESS_NAMESPACES;
        static const char_t* FEATURE_COMPLETLY_LOOSE;

        virtual error_t setInput(Reader& reader) = 0;
        virtual error_t setFeature(const String& feature, bool flag) = 0;
        virtual error_t nextToken(EventType& ret) =0;
        virtual error_t next(EventType& ret) = 0;
        virtual error_t getPositionDescription(String& ret) = 0;
        virtual EventType getEventType() = 0;
        virtual void    defineEntityReplacementText(const String& entity, const String& value) = 0;
        virtual String  getText() = 0;
        virtual bool    getFeature(const String& feature) = 0;
        virtual String  getInputEncoding() = 0;
        virtual String  getNamespacePrefix(int pos) = 0;
        virtual String  getNamespaceUri(int pos) = 0;
        virtual int     getDepth() = 0;
        virtual int     getLineNumber() = 0;
        virtual int     getColumnNumber() = 0;
        virtual error_t isWhitespace(bool& ret) = 0;
        virtual String  getNamespace() = 0;
        virtual String  getName() = 0;
        virtual String  getPrefix() = 0;
        virtual error_t isEmptyElementTag(bool& ret) = 0;
        virtual int     getAttributeCount() = 0;
        virtual error_t getAttributeNamespace(String& ret, int index) = 0;
        virtual error_t getAttributeName(String& ret, int index) = 0;
        virtual error_t getAttributePrefix(String& ret, int index) = 0;
        virtual error_t getAttributeValue(String& ret, int index) = 0;
        virtual String  getAttributeValue(const String& nameSpace, const String& name) = 0;
        virtual String  getNamespace(const String& prefix) = 0;
        virtual error_t getNamespaceCount(int& ret, int depth) = 0;
        virtual error_t nextText(String& ret) = 0;
        virtual error_t nextTag(EventType& ret) = 0;
        
        virtual ~XmlPullParser() {}
    };
}

#endif
