#ifndef __KXML2_XML_PULL_HPP__
#define __KXML2_XML_PULL_HPP__

#include "KXml2.hpp"
#include "XmlReader.hpp"
#include <vector>

// Defines a public, abstract interface for pull XML parsing
// This api is based on Java's XmlPull API (http://www.xmlpull.org/)
namespace KXml2{
    class XmlPullParser
    {
        public:
            static const int START_DOCUMENT = 0;
            static const int START_TAG = 2;
            static const int TEXT = 4;
            static const int END_TAG = 3;
            static const int END_DOCUMENT = 1;

            static const int ENTITY_REF = 6;
            static const int IGNORABLE_WHITESPACE = 7;

            static const int COMMENT = 9;
            static const int CDSECT = 5;
            static const int DOCDECL = 10;
            static const int PROCESSING_INSTRUCTION = 8;
            
            
            typedef char_t TypeDescription_t[24];
            static const unsigned int typesArrayLength=11;
            static const TypeDescription_t TYPES[typesArrayLength];

            static const char_t* FEATURE_PROCESS_NAMESPACES;

            virtual error_t setInput(XmlReader *reader) = 0;
            virtual error_t setFeature(const String& feature, bool flag) = 0;
            virtual error_t nextToken(int& ret) =0;
            virtual error_t next(int& ret) = 0;
            virtual error_t getPositionDescription(String& ret) = 0;
            virtual int     getEventType() = 0;
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
            virtual error_t nextTag(int& ret) = 0;
    };
}

#endif
