#ifndef __ARSLEXIS_READER_HPP__
#define __ARSLEXIS_READER_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>
#include <Utility.hpp>

class Reader: private NonCopyable
{
public:

    enum {npos=-1};

    /**
     * Reads single character from stream.
     * @param chr on succesful return contains read character or @c Reader::npos if end of stream is encountered.
     * @return status code, @c errNone on success.
     */
    status_t read(int& chr);
    
    /**
     * Reads up to @c range characters from stream into buffer @c dst starting from position @c offset.
     * @param num on successful return contains actual number of read characters or @c Reader::npos if you trying to read past end of stream.
     * @return status code, @c errNone on success.
     */
    status_t read(char_t* buffer, uint_t& length);

    // return 0 in length if this is the end of stream
    virtual status_t readRaw(void* buffer, uint_t& length)=0;
    
    virtual status_t readLine(bool& eof, String& out, char_t delimiter=_T('\n'));
    
    virtual ~Reader();
    
};

#endif
