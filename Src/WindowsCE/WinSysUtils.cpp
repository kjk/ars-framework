#include <SysUtils.hpp>

#include <Shellapi.h>

using ArsLexis::String;

// TODO: implement. Used by iPediaConnection
ulong_t ArsLexis::random(ulong_t range)
{
    ulong_t result = Random();
    return result;
}

void ArsLexis::sendEvent(uint_t event, const void* data, uint_t dataSize, bool unique)
{
    assert(dataSize<=sizeof(EventData));
    EventData i;
    memcpy(&i, data, dataSize);
    PostMessage(HWND_BROADCAST, event, i.wParam, i.lParam);
}

void ArsLexis::sendEvent(uint_t event, short wph, short wpl, int lp)
{
    PostMessage(HWND_BROADCAST, event, (wph<<16)|wpl, lp);
}

void ArsLexis::localizeNumber(char_t* begin, char_t* end)
{
   
}

void ArsLexis::processReadyUiEvents()
{
	
}

// return a text in text edit window represented by hwnd in txtOut
void GetEditWinText(HWND hwnd, ArsLexis::String &txtOut)
{
    // 128 should be enough for anybody
    TCHAR buf[128];

    ZeroMemory(buf,sizeof(buf));

    int len = SendMessage(hwnd, EM_LINELENGTH, 0,0)+1;

    if (len < sizeof(buf)/sizeof(buf[0]) )
    {
        len = SendMessage(hwnd, WM_GETTEXT, len, (LPARAM)buf);
        txtOut.assign(buf);
    }
    else
        txtOut.clear();
}

void SetEditWinText(HWND hwnd, ArsLexis::String& txt)
{
    SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)txt.c_str());
}

// try to launch built-in web browser with a given url
bool GotoURL(const ArsLexis::char_t *url)
{
    SHELLEXECUTEINFO sei;
    ZeroMemory(&sei, sizeof(sei));
    sei.cbSize  = sizeof(sei);
    sei.fMask   = SEE_MASK_FLAG_NO_UI;
    sei.lpVerb  = _T("open");
    sei.lpFile  = url;
    sei.nShow   = SW_SHOWMAXIMIZED;

    if (ShellExecuteEx(&sei))
        return true;
    return false;
}

// see http://www.opennetcf.org/Forums/post.asp?method=TopicQuote&TOPIC_ID=95&FORUM_ID=12 for
// more possibilities
// return false on failure, true if ok
bool GetSpecialFolderPath(String& pathOut)
{
#ifdef CSIDL_APPDATA
    // this doesn't seem to be defined on sm 2002
    TCHAR szPath[MAX_PATH];
    BOOL  fOk = SHGetSpecialFolderPath(NULL, szPath, CSIDL_APPDATA, FALSE);
    if (!fOk)
        return false;
    pathOut.assign(szPath);
    return true;
#else
  #ifdef CSIDL_PERSONAL
      // this is defined on sm 2002 and goes to "\My Documents".
      // Not sure if I should use it
      TCHAR szPath[MAX_PATH];
      BOOL  fOk = SHGetSpecialFolderPath(NULL, szPath, CSIDL_PERSONAL, FALSE);
      if (!fOk)
          return false;
      pathOut.assign(szPath);
      return true;
 #else
    // if all else fails, just put it in root "\"
    pathOut.assign(_T(""));
    return true;
 #endif
#endif
}

