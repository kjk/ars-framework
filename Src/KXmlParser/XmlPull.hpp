#ifndef __KXML2_XML_PULL_HPP__
#define __KXML2_XML_PULL_HPP__

#include "XmlReader.hpp"
#include "KXml2.hpp"

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

            virtual void    setInput(XmlReader *reader) = 0;
            virtual void    setFeature(String feature, bool flag) = 0;
            virtual void    nextToken() = 0;
            virtual error_t next(int& ret) = 0;
            virtual String  getPositionDescription() = 0;
            virtual int     getEventType() = 0;
            virtual void    defineEntityReplacementText(String entity, String value) = 0;
    };
}

#endif
