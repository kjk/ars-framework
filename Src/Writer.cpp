#include <Writer.hpp>

#if defined(__MWERKS__)
# pragma inline_depth(100)
# pragma inline_bottom_up on
#endif

namespace ArsLexis {

    status_t Writer::write(const String& str, typename String::size_type startOffset, typename String::size_type length)
    {
        const char_t* data=str.data()+startOffset;
        if (str.npos==length)
            length=str.length()-startOffset;
        while (length)
        {
            status_t error=write(*data++);
            if (errNone!=error)
                return error;
        }
        return errNone;
    }

}