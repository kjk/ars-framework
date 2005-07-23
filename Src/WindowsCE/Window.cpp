#include <WindowsCE/Window.hpp>

Window::Window(AutoDeleteOption ad, bool inputPanel):
	Widget(ad),
	isInputDialog_(inputPanel)
{
#ifdef SHELL_SIP
	ZeroMemory(&activateInfo_, sizeof(activateInfo_));
	activateInfo_.cbSize = sizeof(activateInfo_);
#endif
}

Window::Window(HWND handle, AutoDeleteOption ad, bool inputPanel):
	Widget(handle, ad),
	isInputDialog_(inputPanel)
{
#ifdef SHELL_SIP
	ZeroMemory(&activateInfo_, sizeof(activateInfo_));
	activateInfo_.cbSize = sizeof(activateInfo_);
#endif
}

LRESULT Window::callback(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#ifdef SHELL_SIP
	if (isInputDialog_)
		SHInputDialog(handle(), uMsg, wParam);
#endif

	switch (uMsg) {
		
		case WM_CLOSE:
			return rawHandleClose(uMsg, wParam, lParam);

		case WM_ACTIVATE:
			return rawHandleActivate(uMsg, wParam, lParam);

		case WM_SETTINGCHANGE:
			return rawHandleSettingChange(uMsg, wParam, lParam);

		default:
			return Widget::callback(uMsg, wParam, lParam);
	}
}

LRESULT Window::rawHandleActivate(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ushort action = LOWORD(wParam);
	bool minimized = (0 != HIWORD(wParam));
	HWND prev_handle = reinterpret_cast<HWND>(lParam);
	return handleActivate(action, minimized, prev_handle);
}

long Window::handleActivate(ushort action, bool minimized, HWND previous)
{
	WPARAM wParam = MAKEWPARAM(action, minimized);
	LPARAM lParam = reinterpret_cast<LPARAM>(previous);

#ifdef SHELL_SIP
	DWORD flags = 0;
	if (isInputDialog_)
		flags = SHA_INPUTDIALOG;

	SHHandleWMActivate(handle(), wParam, lParam, &activateInfo_, flags);
#endif

	return defaultCallback(WM_ACTIVATE, wParam, lParam);
}

long Window::handleClose()
{
	return defaultCallback(WM_CLOSE, 0, 0);
}

long Window::handleSettingChange(ulong flag, LPCTSTR section_name)
{
	WPARAM wParam = flag;
	LPARAM lParam = reinterpret_cast<LPARAM>(section_name);

#ifdef SHELL_SIP
	SHHandleWMSettingChange(handle(), wParam, lParam, &activateInfo_);
#endif

	return defaultCallback(WM_SETTINGCHANGE, wParam, lParam);
}