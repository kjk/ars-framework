#include <WindowsCE/Tabs.hpp>
#include <commctrl.h>

TabContainer::TabContainer(AutoDeleteOption ad):
    Widget(ad),
    tabControl_(autoDeleteNot) 
{
} 

TabContainer::TabContainer(HWND tabControlWnd, AutoDeleteOption ad):
    Widget(ad),
    tabControl_(autoDeleteNot) 
{
    attach(tabControlWnd);
} 

TabContainer::~TabContainer()
{
    destroyContainer();
}

void TabContainer::attach(HWND tabControlWnd)
{
    if (valid())
        detach();
         
    tabControl_.attach(tabControlWnd);
    HWND parent = tabControl_.parentHandle();
    Rect r;
    tabControl_.bounds(r);
    if (createContainer(parent, r, NULL))
    { 
        tabControl_.setParent(handle());
        tabControl_.bounds(r);
        r.set(0, 0, r.width(), r.height());
        tabControl_.setBounds(r);
        // TODO: move Z-order
    }
    else
        tabControl_.detach(); 
}

void TabContainer::detach()
{
    if (!valid())
        return;

    assert(tabControl_.valid());
    tabControl_.setParent(parentHandle());
    tabControl_.detach();
    destroyContainer(); 
}

bool TabContainer::createContainer(HWND parent, const Rect& rect, HINSTANCE instance)
{
    if (NULL == instance)
        instance = GetModuleHandle(NULL);
         
	HWND handle = CreateWindowEx(0, TEXT("STATIC"), NULL, WS_CHILD | WS_VISIBLE, rect.x(), rect.y(), rect.width(), rect.height(), parent, NULL, instance, this);
	if (NULL == handle)
	    return false;
	    
    Widget::attach(handle);
    return true; 
}

void TabContainer::destroyContainer()
{
    if (!valid())
        return;
         
    AutoDeleteOption ad = setAutoDelete(autoDeleteNot);
    destroy();
    setAutoDelete(ad);  
}

bool TabContainer::create(DWORD style, int x, int y, int width, int height, HWND parent, HINSTANCE instance)
{
    Rect r(x, y, width, height);
    if (!createContainer(parent, r, instance))
        return false;
    
    if (!tabControl_.create(WC_TABCONTROL, NULL, style, x, y, width, height, handle(), NULL, instance))
    {
        destroyContainer();
        return false; 
    }
    return true; 
}

LRESULT TabContainer::callback(UINT msg, WPARAM wParam, LPARAM lParam)
{
    return Widget::callback(msg, wParam, lParam); 
}