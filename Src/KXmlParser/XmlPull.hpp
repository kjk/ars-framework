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
            static const int END_DOCUMENT = 1;
            static const int START_DOCUMENT = 2;
    	    virtual void setInput(XmlReader *reader) = 0;
            virtual void setFeature(String feature, bool flag) = 0;
            virtual void nextToken() = 0;
            virtual void next() = 0;
            virtual String getPositionDescription() = 0;
            virtual int getEventType() = 0;
    };
}

#endif
