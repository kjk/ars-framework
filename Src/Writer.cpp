#include <Writer.hpp>

#if defined(__MWERKS__)
//# pragma inline_depth(100)
//# pragma inline_bottom_up on
#endif

namespace ArsLexis {

    status_t Writer::write(const char_t* begin, const char_t* end)
    {
        while (begin!=end)
        {
            status_t error=write(*begin++);
            if (errNone!=error)
                return error;
        }
        return errNone;
    }
    
    Writer::~Writer()
    {}

}