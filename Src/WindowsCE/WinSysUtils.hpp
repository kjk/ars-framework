#ifndef __ARSLEXIS_WINSYSUTILS_HPP__
#define __ARSLEXIS_WINSYSUTILS_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>

namespace ArsLexis
{
    ulong_t random(ulong_t range);
    
    void sendEvent(uint_t event, short wph, short wpl, int lp);
    struct EventData {LPARAM lParam; WPARAM wParam;};
}

using ArsLexis::String;

void GetEditWinText(HWND hwnd, String &txtOut);
void SetEditWinText(HWND hwnd, const String& txt);
bool FGetListSelectedItemText(HWND ctrl, String& txtOut);
bool GotoURL(const ArsLexis::char_t *url);
bool GetSpecialFolderPath(String& pathOut, BOOL fCreate=TRUE);

#endif
