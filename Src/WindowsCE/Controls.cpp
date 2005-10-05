#include <WindowsCE/Controls.hpp>

static BOOL InitCC(DWORD control)
{
    INITCOMMONCONTROLSEX icc;
    ZeroMemory(&icc, sizeof(icc));
    icc.dwSize = sizeof(icc);
    icc.dwICC = control;   
    return InitCommonControlsEx(&icc); 
}

#define DEFINE_CONTROL_CTORS(ControlClass) \
ControlClass::ControlClass(AutoDeleteOption ad): Widget(ad) {} \
ControlClass::ControlClass(HWND wnd, AutoDeleteOption ad): Widget(wnd, ad) {} \
ControlClass::~ControlClass() {}


#define DEFINE_CONTROL_CREATE(ControlClass, CLASS) \
bool ControlClass::create(DWORD style, int x, int y, int width, int height, HWND parent, HINSTANCE instance, UINT controlId) \
{ \
    assert(parent != NULL); \
    style |= WS_CHILD; \
    return Widget::create(CLASS, NULL, style, x, y, width, height, parent, (HMENU)controlId, instance); \
} \
bool ControlClass::create(DWORD style, const RECT& rect, HWND parent, HINSTANCE instance, UINT controlId) \
{ \
    assert(parent != NULL); \
    style |= WS_CHILD; \
    return Widget::create(CLASS, NULL, style, rect, parent, (HMENU)controlId, instance); \
}

#define DEFINE_CONTROL_ALL(ControlClass, CLASS) \
    DEFINE_CONTROL_CTORS(ControlClass) \
    DEFINE_CONTROL_CREATE(ControlClass, CLASS)

DEFINE_CONTROL_ALL(ScrollBar, WINDOW_CLASS_SCROLLBAR)

DEFINE_CONTROL_CTORS(EditBox)

bool EditBox::create(DWORD style, int x, int y, int width, int height, HWND parent, HINSTANCE instance, const char_t* text)
{
    assert(parent != NULL);
    style |= WS_CHILD;
    return Widget::create(WINDOW_CLASS_EDITBOX, text, style, x, y, width, height, parent, NULL, instance);
}

ulong_t EditBox::charAtPoint(const Point& p, ulong_t* line) const
{
    LRESULT res = sendMessage(EM_CHARFROMPOS, 0, MAKELPARAM(p.x, p.y));
    if (NULL != line)
        *line = HIWORD(res);
    return LOWORD(res);
}

DEFINE_CONTROL_ALL(ProgressBar, PROGRESS_CLASS)

DEFINE_CONTROL_ALL(TabControl, WINDOW_CLASS_TABCONTROL)

DEFINE_CONTROL_ALL(ListView, WINDOW_CLASS_LISTVIEW)

DEFINE_CONTROL_ALL(Button, WINDOW_CLASS_BUTTON)

DEFINE_CONTROL_ALL(ListBox, WINDOW_CLASS_LISTBOX)

DEFINE_CONTROL_ALL(ComboBox, WINDOW_CLASS_COMBOBOX)

bool ComboBox::setDroppedRect(const RECT& r)
{
    COMBOBOXINFO info = {sizeof(info)};
    if (0 == sendMessage(CB_GETCOMBOBOXINFO, 0, (LPARAM)&info))
        return false;

    if (NULL == info.hwndList)
        return false;

    return FALSE != MoveWindow(info.hwndList, r.left, r.top, r.right - r.left, r.bottom - r.top, FALSE); 
}

DEFINE_CONTROL_ALL(TrackBar, WINDOW_CLASS_TRACKBAR);
