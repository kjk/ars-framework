#include <WindowsCE/CommandBar.hpp>

#ifdef SHELL_MENUBAR 

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

HMENU CommandBar::menuHandle()
{
#ifndef SHGetMenu
#define SHGetMenu(hWndMB) (HMENU)SendMessage((hWndMB), \
        SHCMBM_GETMENU, (WPARAM)0, (LPARAM)0);
#endif
    HMENU menu = SHGetMenu(handle());
    return menu;

/*
	NMNEWMENU nm;
	ZeroMemory(&nm, sizeof(nm));
	LPARAM lParam = reinterpret_cast<LPARAM>(&nm);
	LRESULT res = sendMessage(SHCMBM_GETMENU, 0, lParam);
	return reinterpret_cast<HMENU>(res);
 */	
}

#endif // SHELL_MENUBAR