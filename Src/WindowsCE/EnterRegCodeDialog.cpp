#include <aygshell.h>
#include <WinSysUtils.hpp>
#include <EnterRegCodeDialog.hpp>
#include "resource.h"

/*
This code assumes that there is resource.h file in the project
that uses this file and that IDR_REGISTER_MENUBER,IDC_EDIT_REGCODE
are defined there and IDC_EDIT_REGCODE is an appropriate dialog resource.
Look into sm_inoah and sm_ipedia to see how it's defined.
*/

#define REGISTER_PRESSED 1
#define LATER_PRESSED    2

using ArsLexis::String;

static String g_oldRegCode;
static String g_newRegCode;

static BOOL OnInitDialog(HWND hDlg)
{
    SHINITDLGINFO shidi;
    ZeroMemory(&shidi, sizeof(shidi));
    shidi.dwMask   = SHIDIM_FLAGS;
    shidi.dwFlags  = SHIDIF_SIZEDLGFULLSCREEN;
    shidi.hDlg     = hDlg;

    SHMENUBARINFO shmbi;
    ZeroMemory(&shmbi, sizeof(shmbi));
    shmbi.cbSize     = sizeof(shmbi);
    shmbi.hwndParent = hDlg;
    shmbi.nToolBarId = IDR_REGISTER_MENUBAR;
    shmbi.hInstRes   = GetModuleHandle(NULL);

    if (!SHInitDialog(&shidi))
        return FALSE;

    if (!SHCreateMenuBar(&shmbi))
        return FALSE;

    (void)SendMessage(shmbi.hwndMB, SHCMBM_OVERRIDEKEY, VK_TBACK,
        MAKELPARAM(SHMBOF_NODEFAULT | SHMBOF_NOTIFY,
        SHMBOF_NODEFAULT | SHMBOF_NOTIFY));

    HWND hwndEdit = GetDlgItem(hDlg,IDC_EDIT_REGCODE);

    SetEditWinText(hwndEdit, g_oldRegCode);
    SendMessage(hwndEdit, EM_SETINPUTMODE, 0, EIM_NUMBERS);

    SetFocus(hwndEdit);
    return TRUE;
}

static BOOL CALLBACK RegCodeDlgProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp)
{
    if (WM_INITDIALOG==msg)
    {
        return OnInitDialog(hDlg);
    }

    if (WM_COMMAND==msg)
    {
        assert( (IDM_LATER==wp) || (IDM_REGISTER==wp));

        if (IDM_LATER==wp)
        {
            EndDialog(hDlg, LATER_PRESSED);
            return TRUE;
        }
        else if (IDM_REGISTER==wp)
        {
            HWND hwndEdit = GetDlgItem(hDlg,IDC_EDIT_REGCODE);
            GetEditWinText(hwndEdit, g_newRegCode);
            if (!g_newRegCode.empty())
            {
                EndDialog(hDlg, REGISTER_PRESSED);
            }
            return TRUE;
        }
    }
    return FALSE;
}

bool FGetRegCodeFromUser(HWND hwnd, const String& currentRegCode, String& newRegCodeOut)
{
    g_oldRegCode.assign(currentRegCode);

    int result = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_REGISTER), hwnd, RegCodeDlgProc);

    if (REGISTER_PRESSED==result)
    {
        newRegCodeOut.assign(g_newRegCode);
        return true;
    }
    else
        return false;
}

