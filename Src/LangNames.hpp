#ifndef ARSLEXIS_LANG_NAMES_HPP__
#define ARSLEXIS_LANG_NAMES_HPP__

#include <BaseTypes.hpp>

//const char_t* GetLangNameByLangCode(const String& langCode);

// Finds language name given its ISO 639 2-character code.
// If unknown, returns NULL.
// Note: code doesn't have to be null-terminated, only 2 first characters are checked (case-insensitive).
const char_t* GetLangNameByLangCode(const char_t* code);

#endif