#ifndef __KXML2_XML_READER_HPP__
#define __KXML2_XML_READER_HPP__

#include "KXml2.hpp"

// Defines a public, abstract interface for a class that provides
// input for XmlPullParser. We use our own abstraction, instead of using
// built in C++ functionality, because it's a simple class and we want to
// provide good Palm implementation
namespace KXml2{

    class XmlReader
    {
    public:
    
        virtual error_t read(int& ret) = 0;
        
        virtual error_t read(int& ret, String& dst, int offset, int range)=0;
        
        virtual ~XmlReader() {}
        
    	// TODO: and probably some more
    };
    
}
#endif
