#include <SysUtils.hpp>

#include <Shellapi.h>

using ArsLexis::String;
using ArsLexis::char_t;

// TODO: implement. Used by iPediaConnection
ulong_t ArsLexis::random(ulong_t range)
{
    ulong_t result = Random();
    return result;
}

// must be defined elsewhere e.g. iPediaApplication.cpp
extern HWND g_hwndForEvents;

void ArsLexis::sendEvent(uint_t event, const void* data, uint_t dataSize, bool unique)
{
    assert(dataSize<=sizeof(EventData));
    EventData i;
    memcpy(&i, data, dataSize);
    PostMessage(g_hwndForEvents, event, i.wParam, i.lParam);
}

void ArsLexis::sendEvent(uint_t event, short wph, short wpl, int lp)
{
    PostMessage(g_hwndForEvents, event, (wph<<16)|wpl, lp);
}

void ArsLexis::localizeNumber(char_t* begin, char_t* end)
{
   
}

void ArsLexis::processReadyUiEvents()
{

}

// Put the text of currently selected item in list control into txtOut
// return false if no item is currently selected 
bool ListCtrlGetSelectedItemText(HWND ctrl, String& txtOut)
{
    int idx = SendMessage(ctrl, LB_GETCURSEL, 0, 0);
    if (-1==idx)
    {
        txtOut.clear();
        return false;
    }
    int len = SendMessage(ctrl, LB_GETTEXTLEN, idx, 0);
    char_t *buf = new char_t[len+1];
    ZeroMemory(buf, sizeof(char_t)*(len+1));
    SendMessage(ctrl, LB_GETTEXT, idx, (LPARAM) buf);
    txtOut.assign(buf);
    delete [] buf;
    return true;
}

// return a text in text edit window represented by hwnd in txtOut
void GetEditWinText(HWND hwnd, ArsLexis::String &txtOut)
{
    // 128 should be enough for anybody
    TCHAR buf[128] = {0};

    //ZeroMemory(buf,sizeof(buf));

    int len = SendMessage(hwnd, EM_LINELENGTH, 0,0)+1;

    if (len < sizeof(buf)/sizeof(buf[0]) )
    {
        len = SendMessage(hwnd, WM_GETTEXT, len, (LPARAM)buf);
        txtOut.assign(buf);
    }
    else
        txtOut.clear();
}

void SetEditWinText(HWND hwnd, const ArsLexis::String& txt)
{
    SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)txt.c_str());
}

void ListCtrlFillFromList(HWND ctrlList, CharPtrList_t& strList, bool fRemoveDups)
{
    CharPtrList_t::iterator iter = strList.begin();
    CharPtrList_t::iterator iterEnd = strList.end();

    const char_t *str;
    do {
        str = *iter;
        if (fRemoveDups)
        {
            int idx = SendMessage(ctrlList, LB_FINDSTRINGEXACT, 0, (LPARAM)str);
            if (LB_ERR == idx)
                SendMessage(ctrlList, LB_ADDSTRING, 0, (LPARAM)str);
        }
        else
        {
            SendMessage(ctrlList, LB_ADDSTRING, 0, (LPARAM)str);
        }
        iter++;
    } while (iter!=iterEnd);
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

// On windows those are defined as:
//#define CSIDL_PROGRAMS           0x0002
//#define CSIDL_PERSONAL           0x0005
//#define CSIDL_APPDATA              0x001a
// see shlobj.h for more

#ifdef CSIDL_APPDATA
// this doesn't seem to be defined on sm 2002
#define SPECIAL_FOLDER_PATH CSIDL_APPDATA
#endif


//#ifdef CSIDL_PROGRAMS
// this is "\Windows\Start Menu\Programs" on sm 2002 emu
// #ifndef SPECIAL_FOLDER_PATH
//  #define SPECIAL_FOLDER_PATH CSIDL_PROGRAMS
// #endif
//#endif

#ifdef CSIDL_PERSONAL
// this is defined on sm 2002 and goes to "\My Documents".
// Not sure if I should use it
 #ifndef SPECIAL_FOLDER_PATH
  #define SPECIAL_FOLDER_PATH CSIDL_PERSONAL
 #endif
#endif

// see http://www.opennetcf.org/Forums/post.asp?method=TopicQuote&TOPIC_ID=95&FORUM_ID=12 for
// more possibilities
// return false on failure, true if ok. Even if returns false, it'll return root ("\")
// directory so that clients can ignore failures from this function
bool GetSpecialFolderPath(String& pathOut, BOOL fCreate)
{
#ifdef SPECIAL_FOLDER_PATH
    TCHAR szPath[MAX_PATH];
    int nFolder = SPECIAL_FOLDER_PATH;
    BOOL  fOk = SHGetSpecialFolderPath(NULL, szPath, nFolder, fCreate);
    if (fOk)
    {
        pathOut.assign(szPath);
        return true;
    }
    else
    {
        pathOut.assign(_T(""));
        return false;
    }
#else
    // if all else fails, just use root ("\") directory
    pathOut.assign(_T(""));
    return true;
#endif
}

