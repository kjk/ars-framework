#include <SysUtils.hpp>

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

