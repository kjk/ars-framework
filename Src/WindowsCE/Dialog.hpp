#ifndef GUI_DIALOG_H__
#define GUI_DIALOG_H__

#include <WindowsCE/Window.hpp>

class Dialog: public Window
{
#ifdef SHELL_AYGSHELL
	DWORD initDialogFlags_;
#endif

public:
	
	explicit Dialog(AutoDeleteOption ad = autoDeleteNot, bool inputDialog = false, DWORD initDialogFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN);

	explicit Dialog(HWND wnd, AutoDeleteOption ad = autoDeleteNot, bool inputDialog = false);
	
	~Dialog();
	
	void setInitFlags(DWORD flags) {initDialogFlags_ = flags;}

	bool create(HINSTANCE instance, LPCTSTR resource_id, HWND parent);

	bool create(HINSTANCE instance, UINT resource_id, HWND parent) {return create(instance, MAKEINTRESOURCE(resource_id), parent);}
	
	long showModal(HINSTANCE instance, LPCTSTR resource_id, HWND parent);
	
	long showModal(HINSTANCE instance, UINT resource_id, HWND parent) {return showModal(instance, MAKEINTRESOURCE(resource_id), parent);}

protected:

	virtual bool handleInitDialog(HWND focus_widget_handle, long init_param);
	
	LRESULT callback(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:

	BOOL rawHandleInitDialog(UINT message, WPARAM wParam, LPARAM lParam);

private:

	static BOOL CALLBACK dialogCallback(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

	static BOOL CALLBACK defaultDialogCallback(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

}; // class Dialog

#endif // GUI_DIALOG_H__