#include "Utility.hpp"
#include <cctype>

namespace ArsLexis 
{

    bool startsWith(const String& text, const char_t* start, uint_t startOffset)
    {
        while (startOffset<text.length() && *start)
        {
            if (*start==text[startOffset])
            {
                ++start;
                ++startOffset;
            }
            else
                return false;
        }
        return 0==*start;
    }
    
    bool startsWith(const String& text, const String& start, uint_t startOffset)
    {
        uint_t pos=0;
        while (startOffset<text.length() && pos<start.length())
        {
            if (text[startOffset]==start[pos])
            {
                ++startOffset;
                ++pos;
            }
            else
                return false;
        }
        return pos==start.length();
    }
    
    bool startsWithIgnoreCase(const String& text, const char_t* start, uint_t startOffset)
    {
        while (startOffset<text.length() && *start)
        {
            if (std::tolower(*start)==std::tolower(text[startOffset]))
            {
                ++start;
                ++startOffset;
            }
            else
                return false;
        }
        return 0==*start;
    }
    
    bool equalsIgnoreCase(const char_t* s1start, const char_t* s1end, const char_t* s2start, const char_t* s2end)
    {
        while (s1start!=s1end && s2start!=s2end)
        {
            if (std::tolower(*s1start)==std::tolower(*s2start))
            {
                ++s1start;
                ++s2start;
            }
            else 
                return false;
        }
        return (s1start==s1end && s2start==s2end);
    }
    
    
}

