#include <Window.hpp>
#include <Geometry.hpp>

#ifndef NDEBUG
WinHandle Window::handle() const
{
    assert(valid());
    return handle_;
}
#endif

Window::~Window()
{
    if (valid())
        dispose();
}

Err Window::create(const Rectangle& bounds, FrameType frame, bool modal, bool focusable)
{
    assert(!valid());
    RectangleType rect = toNative(bounds);
    Err error;
    WinHandle h = WinCreateWindow(&rect, frame, modal, focusable, &error);
    if (errNone == error)
        handle_ = h;
    return error;
}

void Window::dispose(EraseOption erase) 
{
    WinDeleteWindow(handle(), bool(erase));
    handle_ = NULL;
}
    
