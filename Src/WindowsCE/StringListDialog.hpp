#ifndef _STRING_LIST_DLG_H_
#define _STRING_LIST_DLG_H_

#include <Text.hpp>
#include <windows.h>

using ArsLexis::String;

bool FGetStringFromList(HWND hwnd, CharPtrList_t& strList, String& strOut);
bool FGetStringFromListRemoveDups(HWND hwnd, CharPtrList_t& strList, String& strOut);

#endif
