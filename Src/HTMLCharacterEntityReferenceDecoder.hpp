#ifndef __ARSLEXIS_HTML_CHARACTER_ENTITY_REFERENCE_DECODER_HPP__
#define __ARSLEXIS_HTML_CHARACTER_ENTITY_REFERENCE_DECODER_HPP__

#include "Debug.hpp"
#include "Utility.hpp"
#include <map>

namespace ArsLexis 
{

    class HTMLCharacterEntityReferenceDecoder
    {
        typedef std::map<const char*, unsigned char, ArsLexis::C_StringLess > Map_t;
        Map_t map_;
        
        void insert(const char* key, unsigned char value);
        
    public:
    
        HTMLCharacterEntityReferenceDecoder();

        /**
         * @return decoded character on success, @c chrNull on failure.
         */
        char decode(const char* entityReference) const;
    };

}

#endif