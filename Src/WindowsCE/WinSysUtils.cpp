#include <windows.h>

#ifdef WIN32_PLATFORM_WFSP
#include <tpcshell.h>
#include <winuserm.h> // for VK_TBACK
#endif

#include <aygshell.h>

#include <Shellapi.h>
#include "SysUtils.hpp"

#include <BaseTypes.hpp>

using ArsLexis::String;
using ArsLexis::char_t;

int RectDx(RECT *rect)
{
    int dx = rect->right - rect->left;
    return dx;
}

int RectDy(RECT *rect)
{
    int dy = rect->bottom - rect->top;
    return dy;
}

// TODO: implement. Used by iPediaConnection
ulong_t random(ulong_t range)
{
    ulong_t result = Random();
    return result;
}

// must be defined elsewhere e.g. iPediaApplication.cpp
extern HWND g_hwndForEvents;

void sendEvent(uint_t event, const void* data, uint_t dataSize, bool unique, const Point*)
{
	// TODO: do something to store Point
    assert(dataSize <= sizeof(EventData));
    EventData i;
    memcpy(&i, data, dataSize);
    PostMessage(g_hwndForEvents, event, i.wParam, i.lParam);
}

void sendEvent(uint_t event, short wph, short wpl, int lp)
{
    PostMessage(g_hwndForEvents, event, (wph<<16)|wpl, lp);
}

void localizeNumber(char_t* begin, char_t* end)
{
	// TODO: implement localizeNumber()   
}

// Put the text of currently selected item in list control into txtOut
// return false if no item is currently selected 
bool ListCtrlGetSelectedItemText(HWND ctrl, String& txtOut)
{
    int idx = SendMessage(ctrl, LB_GETCURSEL, 0, 0);
    if (-1 == idx)
    {
        txtOut.clear();
        return false;
    }
    int len = SendMessage(ctrl, LB_GETTEXTLEN, idx, 0);
	txtOut.resize(len + 1);
    SendMessage(ctrl, LB_GETTEXT, idx, (LPARAM) &txtOut[0]);
	txtOut.resize(len);
    return true;
}

// return a text in text edit window represented by hwnd in txtOut
void GetEditWinText(HWND hwnd, String& txtOut)
{
    int len = SendMessage(hwnd, EM_LINELENGTH, 0, 0);
	txtOut.resize(len + 1);
	len = SendMessage(hwnd, WM_GETTEXT, len + 1, (LPARAM)&txtOut[0]);	
	txtOut.resize(len);
}

void SetEditWinText(HWND hwnd, const String& txt)
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
        ++iter;
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

int GetScrollBarDx()
{
    static int scrollBarDx = -1;
    if (-1==scrollBarDx)
    {
        scrollBarDx = GetSystemMetrics(SM_CXVSCROLL);
    }
    return scrollBarDx;
}

///The following Defines are only on Smartphone and Pocket PC Phone Edition
#if (_WIN32_WCE>=300 || WIN32_PLATFORM_WFSP )

// those 3 must be in this sequence in order to get IID_DestNetInternet
// http://www.smartphonedn.com/forums/viewtopic.php?t=360
#include <objbase.h>
#include <initguid.h>
#include <connmgr.h>

typedef HRESULT (*CMGR_CONNECTION_STATUS)(HANDLE hConnection,DWORD *pdwStatus);
typedef HRESULT (*CMGR_RELEASE_CONN)(HANDLE hConnection,LONG lCache );
typedef HRESULT (*CMGR_ESTABLISH_CONN)(CONNMGR_CONNECTIONINFO *pConnInfo, HANDLE *phConnection, DWORD dwTimeout, DWORD *pdwStatus);

CMGR_CONNECTION_STATUS  g_hConnMgrConnectionStatus = NULL;
CMGR_ESTABLISH_CONN     g_hConnMgrEstablishConnectionSync = NULL;
CMGR_RELEASE_CONN       g_hConnMgrReleaseConnection = NULL;

// It is good practice to load the cellcore.dll dynamically to be able to
// compile the code even for older platforms
// Load cellcore.dll dynamically. Return false if couldn't do it
bool InitCellCore()
{
    static bool fInited = false;
    if (fInited)
    {
        if (NULL==g_hConnMgrConnectionStatus)
            return false;
        else
            return true;
    }

    fInited = true;
    HINSTANCE   hCellDll = LoadLibrary(TEXT("cellcore.dll"));
    if (NULL==hCellDll)
        return false;

    DWORD lastError;
    SetLastError(0);
    // We need the Status and a call to establish the 
    // connection
    g_hConnMgrConnectionStatus = (CMGR_CONNECTION_STATUS)GetProcAddress(hCellDll,TEXT("ConnMgrConnectionStatus")); 
    // The next line is just for debugging. You will have
    // to decide what you want to do if this call fails
    lastError = GetLastError();
    if (0!=lastError)
        goto WasError;
    g_hConnMgrEstablishConnectionSync = (CMGR_ESTABLISH_CONN)GetProcAddress(hCellDll,TEXT("ConnMgrEstablishConnectionSync")); 
    lastError = GetLastError();
    if (0!=lastError)
        goto WasError;
    g_hConnMgrReleaseConnection = (CMGR_RELEASE_CONN)GetProcAddress(hCellDll,TEXT("ConnMgrReleaseConnection")); 
    lastError = GetLastError();
    if (0!=lastError)
        goto WasError;
    return true;
WasError:
    g_hConnMgrConnectionStatus = NULL;
    g_hConnMgrEstablishConnectionSync = NULL;
    g_hConnMgrReleaseConnection = NULL;

    return false;
}

static HANDLE g_hConnection = NULL;

// return true if ConnMgr* connection has already been established
bool FDataConnectionEstablished()
{
    if (NULL==g_hConnection)
        return false;

    DWORD connStatus;

    (*g_hConnMgrConnectionStatus)(&g_hConnection,&connStatus);

    if (CONNMGR_STATUS_CONNECTED==connStatus)
        return true;

    return false;
}

// try to establish internet connection.
// If can't (e.g. because tcp/ip stack is not working), display a dialog box
// informing about that and return false
// Return true if established connection.
// Can be called multiple times - will do nothing if connection is already established.
bool InitDataConnection()
{
    if (FDataConnectionEstablished())
        return true;

    if (!InitCellCore())
    {
        // if we can't open cellcore.dll then it means it doesn't exist
        // which means that we don't have to use ConnMgr* calls
        // which means that we report success to the caller
        return true;
    }

    CONNMGR_CONNECTIONINFO ccInfo = {0};
    ccInfo.cbSize      = sizeof(ccInfo);
    ccInfo.dwParams    = CONNMGR_PARAM_GUIDDESTNET;
    ccInfo.dwFlags     = CONNMGR_FLAG_PROXY_HTTP;
    ccInfo.dwPriority  = CONNMGR_PRIORITY_USERINTERACTIVE;
    ccInfo.guidDestNet = IID_DestNetInternet;
    
    DWORD dwStatus  = 0;
    DWORD dwTimeout = 5000;     // connection timeout: 5 seconds
    HRESULT res = (*g_hConnMgrEstablishConnectionSync)(&ccInfo, &g_hConnection, dwTimeout, &dwStatus);

    if (FAILED(res))
    {
        assert(NULL==g_hConnection);
        g_hConnection = NULL;
    }

    if (NULL==g_hConnection)
        return false;
    else
        return true;
}

void DeinitDataConnection()
{
    if (NULL != g_hConnection)
    {
        (*g_hConnMgrReleaseConnection)(g_hConnection,1);
        g_hConnection = NULL;
    }
}
#else
#error "wrong SDK"
#endif

// doesn't make much sense to me but this piece of magic comes
// Programmin Windows CE 3rd ed page 841
// the logic seems to be the opposite of the description
// Comment from the book:
// "If the sip is not shown, or showing but not docked, the desktop
// rect deosn't include the height (dy) of the menu bar"
bool FDesktopIncludesMenuBar(SIPINFO *si)
{
    if (!(si->fdwFlags & SIPF_ON) ||
        ((si->fdwFlags & SIPF_ON) && !(si->fdwFlags & SIPF_DOCKED)))
    {
        return true;
    }
    return false;
}

#if 0
bool FDesktopIncludesMenuBarAlternate(SIPINFO *si)
{
    // sip is not on
    if (!(si->fdwFlags & SIPF_ON))
        return true;

    // SIP is on but not docked
    if ( !(si->fdwFlags & SIPF_DOCKED))
        return true;

    return false;
}
#endif

void OverrideBackButton(HWND hwndMenuBar)
{
#ifdef WIN32_PLATFORM_WFSP
    // In order to make Back work properly if there is an edit control,
    // it's necessary to  override it and then call 
    // SHSendBackToFocusWindow(msg, wp, lp) in response to WM_HOTKEY
    // Only needed on smartphone
    (void)SendMessage(
        hwndMenuBar, SHCMBM_OVERRIDEKEY, VK_TBACK,
        MAKELPARAM( SHMBOF_NODEFAULT | SHMBOF_NOTIFY,
                    SHMBOF_NODEFAULT | SHMBOF_NOTIFY)
        );
#else
    // do nothing on Pocket PC
#endif
}

void DrawFancyRectangle(HDC hdc, RECT *rect)
{
    int startX = rect->left;
    int endX   = rect->right;
    int startY = rect->top;
    int endY   = rect->bottom;

    POINT p[2];
    p[0].y = startY;
    p[1].y = endY;

    LOGPEN pen;
    HGDIOBJ origPen = GetCurrentObject(hdc,OBJ_PEN);
    GetObject(origPen, sizeof(pen), &pen);

    int dx = endX - startX;

    float startBlueCol = 255;
    float endBlueCol = 80;

    float blueColSpread = startBlueCol - endBlueCol;

    float spread = blueColSpread/(float)dx;

    float blueColorFlt = startBlueCol;

    int prevBlueColor = -1;
    int curBlueColor = (int)blueColorFlt;

    HPEN newPen;

    for (int x=startX; x<endX; x++)
    {
        p[0].x = x;

        if (curBlueColor!=prevBlueColor)
        {
            pen.lopnColor = RGB(0,0,curBlueColor);
            newPen = CreatePenIndirect(&pen);
            SelectObject(hdc,newPen);
        }

        p[1].x = p[0].x;
        Polyline(hdc, p, 2);

        if (curBlueColor!=prevBlueColor)
        {
            DeleteObject(newPen);
            prevBlueColor = curBlueColor;
        }

        blueColorFlt -= spread;
        curBlueColor = (int)blueColorFlt;
    }

    SelectObject(hdc,origPen);
}

static void DrawRectangle(HDC hdc, RECT *rect)
{
    ::Rectangle(hdc, rect->left, rect->top, rect->right, rect->bottom);
}


