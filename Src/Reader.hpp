#ifndef __ARSLEXIS_READER_HPP__
#define __ARSLEXIS_READER_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>

// Defines a public, abstract interface for a class that provides
// input for XmlPullParser. We use our own abstraction, instead of using
// built in C++ functionality, because it's a simple class and we want to
// provide good Palm implementation
namespace ArsLexis {

    class Reader
    {
    public:
    
        static const int npos=-1;

//  Is there any reasonable semantics for getActPosition() when Reader is based on some abstract stream, say socket?    
//        virtual long getActPosition() = 0;

        virtual status_t read(int& ret) = 0;
        
        virtual status_t read(int& ret, String& dst, int offset, int range)=0;
        
        virtual ~Reader() {}
        
    	// TODO: and probably some more
    };
    
}

#endif
