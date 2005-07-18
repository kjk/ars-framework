#ifndef ARSLEXIS_LANG_NAMES_HPP__
#define ARSLEXIS_LANG_NAMES_HPP__

#include <BaseTypes.hpp>

// Finds language name given its wikipedia language code.
// If unknown, returns NULL.
// Note: code doesn't have to be null-terminated.
const char_t* GetLangNameByLangCode(const char* code, long codeLength);

#ifdef _WIN32_WCE
const wchar_t* GetLangNameByLangCode(const wchar_t* code, long codeLength);
#endif

#endif