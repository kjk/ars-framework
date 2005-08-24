#include <WindowsCE/Controls.hpp>

static BOOL InitCC(DWORD control)
{
    INITCOMMONCONTROLSEX icc;
    ZeroMemory(&icc, sizeof(icc));
    icc.dwSize = sizeof(icc);
    icc.dwICC = control;   
    return InitCommonControlsEx(&icc); 
}

ScrollBar::ScrollBar(AutoDeleteOption ad):
	Widget(ad)
{}

bool ScrollBar::create(DWORD style, int x, int y, int width, int height, HWND parent, HINSTANCE instance)
{
	assert(parent != NULL);
	style |= WS_CHILD;
	return Widget::create(WINDOW_CLASS_SCROLLBAR, NULL, style, x, y, width, height, parent, NULL, instance);
}

EditBox::EditBox(AutoDeleteOption ad):
	Widget(ad)
{}

bool EditBox::create(DWORD style, int x, int y, int width, int height, HWND parent, HINSTANCE instance, const char_t* text, DWORD styleEx)
{
	assert(parent != NULL);
	style |= WS_CHILD;
	return Widget::create(WINDOW_CLASS_EDITBOX, text, style, x, y, width, height, parent, NULL, instance, styleEx);
}

ulong_t EditBox::charAtPoint(const Point& p, ulong_t* line) const
{
    LRESULT res = sendMessage(EM_CHARFROMPOS, 0, MAKELPARAM(p.x, p.y));
    if (NULL != line)
        *line = HIWORD(res);
    return LOWORD(res);
}

ProgressBar::ProgressBar(AutoDeleteOption ad):
    Widget(ad)
{
}

bool ProgressBar::create(DWORD style, int x, int y, int width, int height, HWND parent, HINSTANCE instance)
{
	assert(parent != NULL);
	style |= WS_CHILD;
	return Widget::create(PROGRESS_CLASS, NULL, style, x, y, width, height, parent, NULL, instance);
}



TabControl::TabControl(AutoDeleteOption ad):
    Widget(ad)
{
    InitCC(ICC_TAB_CLASSES);
} 

TabControl::TabControl(HWND wnd, AutoDeleteOption ad):
    Widget(wnd, ad)
{
    InitCC(ICC_TAB_CLASSES);
} 

TabControl::~TabControl()
{
}

bool TabControl::create(DWORD style, int x, int y, int width, int height, HWND parent, HINSTANCE instance, DWORD styleEx)
{
	assert(parent != NULL);
	style |= WS_CHILD;
    return Widget::create(WINDOW_CLASS_TABCONTROL, NULL, style, x, y, width, height, handle(), NULL, instance, styleEx);
}


ListView::ListView(AutoDeleteOption ad):
    Widget(ad)
{
    InitCC(ICC_LISTVIEW_CLASSES);
}

ListView::ListView(HWND wnd, AutoDeleteOption ad):
    Widget(wnd, ad)
{
    InitCC(ICC_LISTVIEW_CLASSES);
}

ListView::~ListView()
{}

bool ListView::create(DWORD style, int x, int y, int width, int height, HWND parent, HINSTANCE instance, DWORD styleEx)   
{
    assert(parent != NULL);
    style |= WS_CHILD;
    return Widget::create(WINDOW_CLASS_LISTVIEW, NULL, style, x, y, width, height, parent, NULL, instance, styleEx);    
}