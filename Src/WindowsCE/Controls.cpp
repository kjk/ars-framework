#include <WindowsCE/Controls.hpp>
#include <commctrl.h>

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