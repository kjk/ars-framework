#include <WindowsCE/CommandBar.hpp>
#include <commctrl.h>
#include <SysUtils.hpp>

#ifdef SHELL_MENUBAR 

#ifndef SHGetMenu
#define SHGetMenu(hWndMB) (HMENU)SendMessage((hWndMB), \
        SHCMBM_GETMENU, (WPARAM)0, (LPARAM)0);
#endif


//#include <newmenu.h>

#if defined(_WIN32_WCE_PSPC)
#include <newmenu.h>
#else
class IPropertyBag;

typedef struct tagNMNEWMENU 
{
    NMHDR hdr;
    TCHAR szReg[80];
    HMENU hMenu;
    CLSID clsid;
    IPropertyBag **pppropbag;
} NMNEWMENU, *PNMNEWMENU;
#endif

CommandBar::CommandBar():
	Widget(autoDeleteNot)
{
}

CommandBar::CommandBar(HWND handle):
	Widget(autoDeleteNot)
{
	attach(handle);
}

CommandBar::~CommandBar()
{
	if (valid())
		detach();
}

void CommandBar::attach(HWND handle)
{
	if (valid())
		detach();
		
	// We can't subclass CommandBar - it uses GWL_USERDATA internally!
	handle_ = handle;
	
	if (valid())
	{
		HMENU m = menuHandle();
		if (NULL != m)
			menuBar_.attach(m);
	}
}

void CommandBar::detach()
{
	if (valid())
	{
		menuBar_.detach();
		handle_ = NULL;
	}
}

bool CommandBar::create(HWND parent, DWORD flags, UINT menu_id, HINSTANCE inst, UINT bitmap_id, UINT bitmap_count, COLORREF background_color)
{
    if (NULL == inst)
        inst = GetModuleHandle(NULL);
         
	SHMENUBARINFO mbi;
	ZeroMemory(&mbi, sizeof(mbi));
	mbi.cbSize = sizeof(mbi);
	mbi.hwndParent = parent;
	mbi.dwFlags = flags;
	mbi.nToolBarId = menu_id;
	mbi.hInstRes   = inst;
	mbi.nBmpId     = bitmap_id;
	mbi.cBmpImages = bitmap_count;
	mbi.clrBk = background_color;
	BOOL res = SHCreateMenuBar(&mbi);
	if (res)
	{
		assert(NULL != mbi.hwndMB);
		attach(mbi.hwndMB);
	}
	
	return valid();
}

void CommandBar::adjustParentSize()
{
	HWND parent = parentHandle();
	if (NULL == parent)
		return;
		
	RECT parent_rect;
	GetWindowRect(parent, &parent_rect);

	RECT this_rect;
	GetWindowRect(handle(), &this_rect);
	
	parent_rect.bottom -= this_rect.bottom - this_rect.top; 
	MoveWindow(parent, parent_rect.left, parent_rect.top, parent_rect.right - parent_rect.left, parent_rect.bottom - parent_rect.top, FALSE);
}

HMENU CommandBar::menuHandle() const  
{
    HMENU menu = SHGetMenu(handle());
    return menu;
}

HMENU CommandBar::subMenu(UINT id) const 
{
    TBBUTTONINFO tbbi = {0};
    tbbi.cbSize = sizeof(tbbi);
    tbbi.dwMask = TBIF_LPARAM | TBIF_STYLE;

    if (-1 == sendMessage(TB_GETBUTTONINFO, id, (LPARAM)&tbbi))
        return NULL;
         
    if (0 == (TBSTYLE_DROPDOWN & tbbi.fsStyle))
        return NULL;
         
    HMENU menu = (HMENU)tbbi.lParam;
    return menu;        
}

bool CommandBar::replaceSubMenu(UINT id, UINT newId, HMENU newMenu, UINT captionId)
{
    TBBUTTONINFO tbbi = {0};
    tbbi.cbSize = sizeof(tbbi);
    tbbi.dwMask = TBIF_LPARAM | TBIF_STYLE;

    if (-1 == sendMessage(TB_GETBUTTONINFO, id, (LPARAM)&tbbi))
        return false;
    
    if (0 == (TBSTYLE_DROPDOWN & tbbi.fsStyle))
        return false;
    
    HMENU prevMenu = (HMENU)tbbi.lParam; 
    char_t* text = LoadString(captionId);
    if (NULL == text)
        return false;
          
    tbbi.dwMask = TBIF_LPARAM | TBIF_COMMAND | TBIF_TEXT;
    tbbi.lParam = (DWORD)newMenu;
    tbbi.idCommand = newId;
    tbbi.pszText = text;
    long res = sendMessage(TB_SETBUTTONINFO, id, (LPARAM)&tbbi);
    free(text);
    if (0 == res)  
        return false;
    DestroyMenu(prevMenu);
    return true;   
}

bool CommandBar::replaceButton(UINT id, UINT newId, UINT captionId)
{
    TBBUTTONINFO tbbi = {0};
    tbbi.cbSize = sizeof(tbbi);
    tbbi.dwMask = TBIF_STYLE;

    if (-1 == sendMessage(TB_GETBUTTONINFO, id, (LPARAM)&tbbi))
        return false;
    
    if (0 != (TBSTYLE_DROPDOWN & tbbi.fsStyle))
        return false;
    
    char_t* text = LoadString(captionId);
    if (NULL == text)
        return false;

    tbbi.dwMask = TBIF_COMMAND | TBIF_TEXT;
    tbbi.idCommand = newId;
    tbbi.pszText = text;
    long res = sendMessage(TB_SETBUTTONINFO, id, (LPARAM)&tbbi);
    free(text);
    if (0 == res)  
        return false;
    return true;
}


#endif // SHELL_MENUBAR