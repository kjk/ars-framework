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

static String oldRegCode;
static String newRegCode;

static BOOL OnInitDialog(HWND hDlg, const String& oldRegCode)
{
    SHINITDLGINFO shidi;
    ZeroMemory(&shidi, sizeof(shidi));
    shidi.dwMask   = SHIDIM_FLAGS;

#ifdef WIN32_PLATFORM_PSPC
        shidi.dwFlags  = SHIDIF_SIZEDLG | SHIDIF_EMPTYMENU;
#else
        shidi.dwFlags  = SHIDIF_SIZEDLGFULLSCREEN;
#endif
    shidi.hDlg     = hDlg;

    if (!SHInitDialog(&shidi))
        return FALSE;

#ifdef WIN32_PLATFORM_WFSP
    SHMENUBARINFO shmbi;
    ZeroMemory(&shmbi, sizeof(shmbi));
    shmbi.cbSize     = sizeof(shmbi);
    shmbi.hwndParent = hDlg;
    shmbi.nToolBarId = IDR_REGISTER_MENUBAR;
    shmbi.hInstRes   = GetModuleHandle(NULL);

    if (!SHCreateMenuBar(&shmbi))
        return FALSE;

    (void)SendMessage(shmbi.hwndMB, SHCMBM_OVERRIDEKEY, VK_TBACK,
        MAKELPARAM(SHMBOF_NODEFAULT | SHMBOF_NOTIFY,
        SHMBOF_NODEFAULT | SHMBOF_NOTIFY));
#endif

    HWND hwndEdit = GetDlgItem(hDlg,IDC_EDIT_REGCODE);

    SetEditWinText(hwndEdit, oldRegCode);
    SendMessage(hwndEdit, EM_SETINPUTMODE, 0, EIM_NUMBERS);

    SetFocus(hwndEdit);

#ifdef WIN32_PLATFORM_PSPC
    SHSipPreference(hDlg, SIP_UP);
#endif

    return TRUE;
}

static BOOL CALLBACK RegCodeDlgProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp)
{
    if (WM_INITDIALOG==msg)
    {
        return OnInitDialog(hDlg, oldRegCode);
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
            GetEditWinText(hwndEdit, newRegCode);
            if (!newRegCode.empty())
            {
                EndDialog(hDlg, REGISTER_PRESSED);
            }
            return TRUE;
        }
    }

#ifdef WIN32_PLATFORM_PSPC
    // TODO: is this really necessary? seems to be working without
    if (WM_SETTINGCHANGE==msg)
    {
        if (SPI_SETSIPINFO == wp)
        {
            SHACTIVATEINFO sai;
            ZeroMemory(&sai, sizeof(sai));
            SHHandleWMSettingChange(hDlg, -1 , 0, &sai);
        }
        return TRUE;
    }

    if (WM_ACTIVATE==msg)
    {
        if (SPI_SETSIPINFO == wp)
        {
            SHACTIVATEINFO sai;
            ZeroMemory(&sai, sizeof(sai));
            SHHandleWMActivate(hDlg, wp, lp, &sai, 0);
        }
        return TRUE;
    }
#endif

    return FALSE;
}

bool FGetRegCodeFromUser(HWND hwnd, const String& currentRegCode, String& newRegCodeOut)
{
    oldRegCode.assign(currentRegCode);

    int result = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_REGISTER), hwnd, RegCodeDlgProc);

    if (REGISTER_PRESSED==result)
    {
        newRegCodeOut.assign(newRegCode);
        return true;
    }
    else
        return false;
}

