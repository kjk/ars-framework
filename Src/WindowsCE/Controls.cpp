#include <WindowsCE/Controls.hpp>

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
