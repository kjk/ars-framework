#ifndef _STRING_LIST_DLG_H_
#define _STRING_LIST_DLG_H_

#include <list>
#include <windows.h>
#include <BaseTypes.hpp>

using ArsLexis::char_t;
using ArsLexis::String;

typedef std::list<const char_t *> StrList_t;

bool FGetStringFromList(HWND hwnd, StrList_t strList, String& strOut);

#endif
