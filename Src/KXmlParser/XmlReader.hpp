#ifndef __KXML2_XML_READER_HPP__
#define __KXML2_XML_READER_HPP__

// Defines a public, abstract interface for a class that provides
// input for XmlPullParser. We use our own abstraction, instead of using
// built in C++ functionality, because it's a simple class and we want to
// provide good Palm implementation
namespace KXml2{
    class XmlReader
    {
    public:
    	virtual int read() = 0;
    	// TODO: and probably some more
    };
}
#endif
