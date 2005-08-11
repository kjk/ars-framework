#include <WindowsCE/Dialog.hpp>

BOOL CALLBACK Dialog::dialogCallback(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (WM_INITDIALOG == uMsg)
	    return Widget::callback(hwndDlg, uMsg, wParam, lParam);
    
    return FALSE; 
}

/*
BOOL CALLBACK Dialog::defaultDialogCallback(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	assert(IsWindow(hwndDlg));
	if (WM_INITDIALOG == uMsg)
	{
		Dialog* d = static_cast<Dialog*>(Widget::fromHandle(hwndDlg));
		assert(NULL != d);
		assert(d->handle() == hwndDlg);
		return d->rawHandleInitDialog(uMsg, wParam, lParam);
	}
	return FALSE;
}
 */
 

Dialog::Dialog(AutoDeleteOption ad, bool inputDialog, DWORD initDialogFlags):
	Window(ad, inputDialog)
#ifdef SHELL_AYGSHELL
	, initDialogFlags_(initDialogFlags)
#endif
{}

Dialog::Dialog(HWND wnd, AutoDeleteOption ad, bool inputDialog):
	Window(wnd, ad, inputDialog)
#ifdef SHELL_AYGSHELL
	, initDialogFlags_(0) // This really doesn't matter - when attaching to existing HWND, WM_INITDLG has already been sent.
#endif
{
}

bool Dialog::create(HINSTANCE inst, LPCTSTR resource_id, HWND parent)
{
	HWND handle = CreateDialogParam(inst, resource_id, parent, dialogCallback, reinterpret_cast<LPARAM>(this));
	return NULL != handle;
}

long Dialog::showModal(HINSTANCE inst, LPCTSTR resId, HWND parent)
{
    return DialogBoxParam(inst, resId, parent, dialogCallback, reinterpret_cast<LPARAM>(this));
}

BOOL Dialog::rawHandleInitDialog(UINT message, WPARAM wParam, LPARAM lParam)
{
	return handleInitDialog(reinterpret_cast<HWND>(wParam), lParam);
}

bool Dialog::handleInitDialog(HWND focus_widget_handle, long init_param)
{
#ifdef SHELL_AYGSHELL
	SHINITDLGINFO idi;
	idi.dwMask = SHIDIM_FLAGS;
	idi.hDlg = handle();
	idi.dwFlags = initDialogFlags_;
	SHInitDialog(&idi);
#endif	
	return messageHandled;
}

LRESULT Dialog::callback(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (WM_INITDIALOG == message)
        return rawHandleInitDialog(message, wParam, lParam);
    return Window::callback(message, wParam, lParam); 
}
