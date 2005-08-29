#include <WindowsCE/Dialog.hpp>
#include <ExtendedEvent.hpp>

//#define DEBUG_DIALOG_CALLBACK_MESSAGES

BOOL CALLBACK Dialog::dialogCallback(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (WM_INITDIALOG == uMsg)
	    return Widget::callback(hwndDlg, uMsg, wParam, lParam);

#ifdef DEBUG_DIALOG_CALLBACK_MESSAGES
    OutputDebugString(TEXT("Dialog::dialogCallback(): "));
    DumpMessage(uMsg, wParam, lParam); 
#endif
    
    return FALSE; 
}

Dialog::Dialog(AutoDeleteOption ad, bool inputDialog, DWORD initDialogFlags):
	Window(ad, inputDialog),
    modal_(false)
#ifdef SHELL_AYGSHELL
	, initDialogFlags_(initDialogFlags)
#endif
{
    setSizeToInputPanel(0 != (SHIDIF_SIZEDLGFULLSCREEN & initDialogFlags));
}

Dialog::Dialog(HWND wnd, AutoDeleteOption ad, bool inputDialog):
	Window(wnd, ad, inputDialog),
	modal_(false)
#ifdef SHELL_AYGSHELL
	, initDialogFlags_(0) // This really doesn't matter - when attaching to existing HWND, WM_INITDLG has already been sent.
#endif
{
}

Dialog::~Dialog()
{
}

bool Dialog::create(HINSTANCE inst, LPCTSTR resource_id, HWND parent)
{
    if (NULL == inst)
        inst = GetModuleHandle(NULL);
         
	HWND handle = CreateDialogParam(inst, resource_id, parent, dialogCallback, reinterpret_cast<LPARAM>(this));
	return NULL != handle;
}

long Dialog::showModal(HINSTANCE inst, LPCTSTR resId, HWND parent)
{
    if (NULL == inst)
        inst = GetModuleHandle(NULL);

    modal_ = true;
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

#ifdef SHELL_SIP
    if (isInputDialog())
        SHSipPreference(handle(), SIP_INPUTDIALOG);
#endif
    
	return true;
}

void Dialog::endModal(int code)
{
    assert(modal_);
    EndDialog(handle(), code);
    modal_ = false; 
}

// #define DEBUG_DIALOG_MESSAGES

LRESULT Dialog::callback(UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT res;
#ifdef DEBUG_DIALOG_MESSAGES
    OutputDebugString(TEXT("Dialog::callback(): "));
    DumpMessage(message, wParam, lParam); 
#endif
    
    if (WM_INITDIALOG == message)
        res = rawHandleInitDialog(message, wParam, lParam);
    else 
        res = Window::callback(message, wParam, lParam); 
    if (modal_ && extEvent == message)
    {
        ExtEventGetID(lParam);
        ExtEventFree(lParam); 
    }
    return res; 
}
