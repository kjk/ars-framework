#ifndef ARSLEXIS_LANG_NAMES_HPP__
#define ARSLEXIS_LANG_NAMES_HPP__

#include <BaseTypes.hpp>

//const char_t* GetLangNameByLangCode(const String& langCode);

// Finds language name given its wikipedia language code.
// If unknown, returns NULL.
// Note: code doesn't have to be null-terminated.
const char_t* GetLangNameByLangCode(const char_t* code, ulong_t codeLength);

#endif