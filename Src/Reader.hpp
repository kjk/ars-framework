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
    
        enum {npos=-1};

        /**
         * Reads single character from stream.
         * @param chr on succesful return contains read character or @c Reader::npos if end of stream is encountered.
         * @return status code, @c errNone on success.
         */
        virtual status_t read(int& chr) = 0;
        
        /**
         * Reads up to @c range characters from stream into buffer @c dst starting from position @c offset.
         * @param num on successful return contains actual number of read characters or @c Reader::npos if you trying to read past end of stream.
         * @return status code, @c errNone on success.
         */
        virtual status_t read(int& num, String& dst, int offset, int range)=0;
        
        status_t readLine(bool& eof, String& out, char_t delimiter=_T('\n'));
        
        virtual ~Reader() {}
        
    };
    
}

#endif
