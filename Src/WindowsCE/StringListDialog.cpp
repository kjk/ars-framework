#include <windows.h>
#include <aygshell.h>
#include <WinSysUtils.hpp>
#include "resource.h"
#include "StringListDialog.hpp"

/*
This is a common dialog to select a string from a list of strings.
A list shouldn't be too long - otherwise it'll be unusable.

The following must be defined in project's resource file:

IDR_STRING_LIST_MENUBAR  - "Select" & "Cancel" menus
IDD_STRING_LIST          - dialog definition
IDC_STRING_LIST          - a list control inside dialog

TODO: build resources dynamically so that we don't need them in resource files.
Makes it easier to use this code.
*/

using ArsLexis::char_t;

static WNDPROC g_oldListWndProc;
static HWND    g_hRecentLookupDlg = NULL;
static bool    fRemoveDups = false;

#define SELECT_PRESSED  1
#define CANCEL_PRESSED  2

static CharPtrList_t g_strList;
static String        selectedWord;

const int hotKeyCode = 0x32;

static LRESULT CALLBACK ListWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (0x87==msg)   //What the hell is constatnt - any idea VK_F24 ??
    {
        if (hotKeyCode==wp)
        {
            SendMessage(g_hRecentLookupDlg, WM_COMMAND, ID_SELECT, 0);
        }
    }
    return CallWindowProc(g_oldListWndProc, hwnd, msg, wp, lp);
}

static BOOL InitStringList(HWND hDlg)
{
    // Specify that the dialog box should stretch full screen
    SHINITDLGINFO shidi = {0};
    shidi.dwMask  = SHIDIM_FLAGS;
    shidi.dwFlags = SHIDIF_SIZEDLGFULLSCREEN;
    shidi.hDlg    = hDlg;

    // Set up the menu bar
    SHMENUBARINFO shmbi = {0};  
    shmbi.cbSize     = sizeof(shmbi);
    shmbi.hwndParent = hDlg;
    shmbi.nToolBarId = IDR_STRING_LIST_MENUBAR;
    shmbi.hInstRes   = GetModuleHandle(NULL);

    // If we could not initialize the dialog box, return an error
    if (!SHInitDialog(&shidi))
        return FALSE;
    
    if (!SHCreateMenuBar(&shmbi))
        return FALSE;
    
    (void)SendMessage(shmbi.hwndMB, SHCMBM_OVERRIDEKEY, VK_TBACK, 
        MAKELPARAM(SHMBOF_NODEFAULT | SHMBOF_NOTIFY, 
        SHMBOF_NODEFAULT | SHMBOF_NOTIFY));
    
    g_hRecentLookupDlg = hDlg;

    HWND ctrl = GetDlgItem(hDlg, IDC_STRING_LIST);
    g_oldListWndProc = (WNDPROC)SetWindowLong(ctrl, GWL_WNDPROC, (LONG)ListWndProc);
    RegisterHotKey(ctrl, hotKeyCode, 0, VK_TACTION);

    const char_t *str;

    CharPtrList_t::iterator iter = g_strList.begin();
    CharPtrList_t::iterator iterEnd = g_strList.end();

    do {
        str = *iter;
        if (fRemoveDups)
        {
            int idx = SendMessage(ctrl, LB_FINDSTRINGEXACT, 0, (LPARAM)str);
            if (LB_ERR == idx)
                SendMessage(ctrl, LB_ADDSTRING, 0, (LPARAM)str);
        }
        else
        {
            SendMessage(ctrl, LB_ADDSTRING, 0, (LPARAM)str);
        }
        iter++;
    } while (iter!=iterEnd);

    SendMessage (ctrl, LB_SETCURSEL, 0, 0);
    return TRUE;
}

static void DoSelect(HWND hDlg)
{
    HWND ctrl = GetDlgItem(hDlg, IDC_STRING_LIST);    
    bool fSelected = FGetListSelectedItemText(ctrl, selectedWord);
    if (fSelected)
        EndDialog(hDlg, SELECT_PRESSED);
}

// TODO: handle SIP
static BOOL CALLBACK StringListDlgProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp)
{
    if (WM_INITDIALOG==msg)
    {
        return InitStringList(hDlg);
    }

    if (WM_SIZE==msg)
    {
        HWND ctrlList = GetDlgItem(hDlg, IDC_STRING_LIST);
        int dlgDx = LOWORD(lp);
        int dlgDy = HIWORD(lp);
        // the idea is that on Pocket PC we have 2 units border on all sides
        // on smartphone we have 2px border only at top and bottom
#ifdef WIN32_PLATFORM_PSPC
        MoveWindow(ctrlList, 2, 2, dlgDx-4, dlgDy-2, TRUE);
#else
        MoveWindow(ctrlList, 0, 2, dlgDx, dlgDy-4, TRUE);
#endif
    }

    if (WM_COMMAND==msg)
    {
        if (wp==ID_CANCEL)
        {
            EndDialog(hDlg, CANCEL_PRESSED);
        }
        else if (wp==ID_SELECT)
        {
            DoSelect(hDlg);
        }
    }
    return FALSE;
}

bool FGetStringFromList(HWND hwnd, CharPtrList_t strList, ArsLexis::String& strOut)
{
    g_strList = strList;
    fRemoveDups = false;
    int result = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_STRING_LIST), hwnd, StringListDlgProc);
    if (SELECT_PRESSED==result)
    {
        strOut.assign(selectedWord);
        return true;
    }

    return false;
}

bool FGetStringFromListRemoveDups(HWND hwnd, CharPtrList_t strList, String& strOut)
{
    g_strList = strList;
    fRemoveDups = true;
    int result = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_STRING_LIST), hwnd, StringListDlgProc);
    if (SELECT_PRESSED==result)
    {
        strOut.assign(selectedWord);
        return true;
    }

    return false;

}

