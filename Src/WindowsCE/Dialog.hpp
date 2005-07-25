#ifndef GUI_DIALOG_H__
#define GUI_DIALOG_H__

#include <WindowsCE/Window.hpp>

class Dialog: public Window
{
#ifdef SHELL_AYGSHELL
	DWORD initDialogFlags_;
#endif

public:
	
	Dialog(AutoDeleteOption ad = autoDeleteNot, bool inputDialog = false, DWORD initDialogFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN);

	Dialog(HWND wnd, AutoDeleteOption ad = autoDeleteNot, bool inputDialog = false);

	bool create(HINSTANCE instance, LPCTSTR resource_id, HWND parent);

	bool create(HINSTANCE instance, UINT resource_id, HWND parent) {return create(instance, reinterpret_cast<LPCTSTR>(resource_id), parent);}

protected:

	virtual bool handleInitDialog(HWND focus_widget_handle, long init_param);

private:

	BOOL rawHandleInitDialog(UINT message, WPARAM wParam, LPARAM lParam);

private:

	static BOOL CALLBACK dialogCallback(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

	static BOOL CALLBACK defaultDialogCallback(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

}; // class Dialog

#endif // GUI_DIALOG_H__