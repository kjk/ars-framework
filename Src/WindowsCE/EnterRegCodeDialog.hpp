#ifndef _ENTER_REG_CODE_H_
#define _ENTER_REG_CODE_H_

#include <windows.h>
#include <BaseTypes.hpp>

bool FGetRegCodeFromUser(HWND hwnd, const ArsLexis::String& currentRegCode, ArsLexis::String& newRegCodeOut);

#endif
