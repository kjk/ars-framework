#include <WindowsCE/Widget.hpp>
#include <WindowsCE/Window.hpp>
#include <WindowsCE/Messages.hpp>
#include <ExtendedEvent.hpp>
#include <Text.hpp>
#include <SysUtils.hpp>

#include <excpt.h>

BOOL ScreenToClient(HWND wnd, RECT& rect)
{
    MapWindowPoints(NULL, wnd, (POINT*)&rect, 2);
    return TRUE;  	
}

BOOL ClientToScreen(HWND wnd, RECT& rect)
{
    MapWindowPoints(wnd, NULL, (POINT*)&rect, 2); 	
    return TRUE;	
}

//#define DEBUG_WIDGET_MESSAGES

void DumpMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    const char_t* name = MessageName(uMsg);
    char_t buffer[12];
    if (NULL != name)
        OutputDebugString(name);
    else 
    {
        tprintf(buffer, TEXT("0x%08x"), uMsg);
        OutputDebugString(buffer);
    }
    if (extEvent != uMsg)
    {
        OutputDebugString(TEXT(" wParam="));
        tprintf(buffer, TEXT("0x%08x"), wParam);
        OutputDebugString(buffer);
        OutputDebugString(TEXT(" lParam="));
        tprintf(buffer, TEXT("0x%08x"), lParam);
        OutputDebugString(buffer);
    }
    else 
    {
        OutputDebugString(TEXT(" id="));
        tprintf(buffer, TEXT("%ld"), ExtEventGetID(lParam));
        OutputDebugString(buffer);
        OutputDebugString(TEXT(" type="));
        tprintf(buffer, TEXT("%ld"), ExtEventGetType(lParam));
        OutputDebugString(buffer);
    }
    if (WM_NOTIFY == uMsg &&  0 != lParam)
    {
        NMHDR* hdr = (NMHDR*)lParam;
        OutputDebugString(TEXT(" notify="));
        name = MessageName(hdr->code);
        if (NULL != name)
            OutputDebugString(name);
        else
        {
            tprintf(buffer, TEXT("0x%08x"), hdr->code);
            OutputDebugString(buffer);
        } 
        OutputDebugString(TEXT(" sender="));
        tprintf(buffer, TEXT("%ld"), hdr->idFrom);
        OutputDebugString(buffer);
    }  
    OutputDebugString(TEXT("\n"));
}

static LRESULT CALLBACK WidgetCallbackDefault(HWND handle, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //OutputDebugString(TEXT("WidgetCallbackDefault(): "));
    //DumpMessage(uMsg, wParam, lParam);

    return DefWindowProc(handle, uMsg, wParam, lParam);
}

static const tchar* widgetCallbackMagicMessageName = TEXT("WidgetCallbackMagicMessage {8bc96a0b-61d9-4e6d-b21a-685cfe07450d}");

static UINT WidgetCallbackMagicMessage()
{
    static UINT wm = RegisterWindowMessage(widgetCallbackMagicMessageName);
    return wm;
}

static const LRESULT widgetCallbackMagicResponse = 0xf6f42e44;

bool IsDialog(HWND handle)
{
    char_t buffer[8];
    int len = GetClassName(handle, buffer, 8);
    if (6 == len && 0 == _tcsicmp(TEXT("DIALOG"), buffer))
        return true;
    return false;
}

bool Widget::isDialog() const 
{
    return IsDialog(handle());
}

static Widget* WidgetFromHandle(HWND handle)
{
    if (NULL == handle)
        return NULL;

    return (Widget*)GetWindowLong(handle, GWL_USERDATA);
}

Widget::Widget(AutoDeleteOption ad):
handle_(NULL),
previousCallback_(NULL),
previousUserData_(NULL),
autoDelete_(ad)
{
}

Widget::Widget(HWND handle, AutoDeleteOption ad):
handle_(NULL),
previousCallback_(NULL),
previousUserData_(NULL),
autoDelete_(ad)
{
    assert(NULL == fromHandle(handle));
    attach(handle);
}

Widget::~Widget()
{
    if (valid())
        detach();
}

void Widget::attach(HWND handle)
{
    if (valid())
        detach();

    assert(NULL != handle);
    handle_ = handle;
    previousUserData_ = GetWindowLong(handle, GWL_USERDATA);
    assert(0 == previousUserData_);

    SetWindowLong(handle, GWL_USERDATA, reinterpret_cast<LONG>(this));

    WNDPROC proc = reinterpret_cast<WNDPROC>(GetWindowLong(handle_, GWL_WNDPROC));

    LRESULT res = CallWindowProc(proc, handle, WidgetCallbackMagicMessage(), 0, 0);
    if (widgetCallbackMagicResponse != res || IsDialog(handle))
    {
        WNDPROC newProc = Widget::callback;
        SetWindowLong(handle_, GWL_WNDPROC, reinterpret_cast<LONG>(newProc));
        previousCallback_ = proc;
    }
}

void Widget::detach()
{
    if (!valid())
        return;

    WNDPROC callback = WidgetCallbackDefault;
    if (NULL != previousCallback_)
        callback = previousCallback_;

    SetWindowLong(handle_, GWL_WNDPROC, reinterpret_cast<LONG>(callback));
    SetWindowLong(handle_, GWL_USERDATA, previousUserData_);
    handle_ = NULL;
}

Widget* Widget::fromHandle(HWND handle, CreateOption co)
{
    Widget* w = WidgetFromHandle(handle);
    if (NULL == w && createWidget == co)
        w = new_nt Widget(handle, autoDelete);
    return w;
}

LRESULT CALLBACK Widget::callback(HWND handle, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

    if (uMsg >= 0xC000 && uMsg <= 0xFFFF)
    {
        UINT magicMessage = WidgetCallbackMagicMessage();
        if (uMsg == magicMessage)
            return widgetCallbackMagicResponse;
    }

#ifdef DEBUG_WIDGET_MESSAGES
    DumpMessage(uMsg, wParam, lParam);
#endif

    //#ifndef NDEBUG
    //    __try 
    //    {
    //#endif    
    Widget* w = WidgetFromHandle(handle);
    if (NULL == w)
    {
        if (WM_CREATE == uMsg)
        {
            CREATESTRUCT* p = (CREATESTRUCT*)lParam;
            assert(NULL != p);
            w = (Widget*)p->lpCreateParams;
        }
        if (WM_INITDIALOG == uMsg)
            w = (Widget*)lParam;

        if (NULL != w)
            w->attach(handle);
    }
    if (NULL == w)
        return WidgetCallbackDefault(handle, uMsg, wParam, lParam);

    return w->callback(uMsg, wParam, lParam);
    //#ifndef NDEBUG
    //    }
    //    __except ((EXCEPTION_BREAKPOINT == GetExceptionCode() || EXCEPTION_SINGLE_STEP == GetExceptionCode()) ? EXCEPTION_CONTINUE_EXECUTION : EXCEPTION_EXECUTE_HANDLER)
    //    {
    //        OutputDebugString(_T("Widget::callback(): exception trapped: "));
    //        static TCHAR buffer[16];
    //        _stprintf(buffer, _T("0x%x; "), GetExceptionCode());
    //        OutputDebugString(buffer);
    //        DumpMessage(uMsg, wParam, lParam);
    //        assert(false);
    //    }
    //    return -1;
    //#endif
}

LRESULT Widget::callback(UINT message, WPARAM wParam, LPARAM lParam)
{
    assert(NULL != handle_);
    switch (message)
    {
    case WM_CREATE:
        return rawHandleCreate(message, wParam, lParam);

    case WM_DESTROY:
        return rawHandleDestroy(message, wParam, lParam);

    case WM_COMMAND:
        return rawHandleCommand(message, wParam, lParam);

    case WM_SIZE:
        return rawHandleResize(message, wParam, lParam);

    case WM_PAINT:
        return rawHandlePaint(message, wParam, lParam);

    case WM_NOTIFY:
        return rawHandleNotify(message, wParam, lParam); 			

    default:
        return defaultCallback(message, wParam, lParam);
    };
}

ATOM Widget::registerClass(UINT style, HINSTANCE instance, HICON icon, HCURSOR cursor, HBRUSH brush, LPCTSTR class_name)
{
    if (NULL == instance)
        instance = GetModuleHandle(NULL);

    WNDCLASS wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.style = style;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(Widget*);
    wc.hbrBackground = brush;
    wc.hCursor = cursor;
    wc.hIcon = icon;
    wc.hInstance = instance;
    wc.lpfnWndProc = Widget::callback;
    wc.lpszClassName = class_name;
    wc.lpszMenuName = NULL;
    return RegisterClass(&wc);
}

bool Widget::create(ATOM widget_class, LPCTSTR caption, DWORD style, int x, int y, int width, int height, HWND parent, HMENU menu, HINSTANCE instance, DWORD exStyle)
{
    return create(reinterpret_cast<LPCTSTR>(widget_class), caption, style, x, y, width, height, parent, menu, instance, exStyle);
}

bool Widget::create(LPCTSTR widget_class, LPCTSTR caption, DWORD style, int x, int y, int width, int height, HWND parent, HMENU menu, HINSTANCE instance, DWORD exStyle)
{
    if (valid())
        detach();

    if (NULL == instance)
        instance = GetModuleHandle(NULL);

    HWND handle = CreateWindowEx(exStyle, widget_class, caption, style, x, y, width, height, parent, menu, instance, this);
    if (NULL == handle)
        return false;

    if (NULL == handle_)
        attach(handle);

    return true;
}

bool Widget::create(LPCTSTR widget_class, LPCTSTR caption, DWORD style, const RECT& rect, HWND parent, HMENU menu, HINSTANCE instance, DWORD exStyle)
{
    return create(widget_class, caption, style, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, parent, menu, instance, exStyle);
}

bool Widget::create(ATOM widget_class, LPCTSTR caption, DWORD style, const RECT& rect, HWND parent, HMENU menu, HINSTANCE instance, DWORD exStyle)
{
    return create(reinterpret_cast<LPCTSTR>(widget_class), caption, style, rect, parent, menu, instance, exStyle);
}

LRESULT Widget::defaultCallback(UINT uMsg, WPARAM wParam, LPARAM lParam, HWND handle)
{
    if (NULL == handle)
        handle = handle_;

    if (NULL != previousCallback_)
    {
        if (0 != previousUserData_)
            SetWindowLong(handle, GWL_USERDATA, previousUserData_);
        LRESULT res = CallWindowProc(previousCallback_, handle, uMsg, wParam, lParam);
        if (0 != previousUserData_)
            SetWindowLong(handle, GWL_USERDATA, reinterpret_cast<LONG>(this));
        return res;
    }
    else
        return DefWindowProc(handle, uMsg, wParam, lParam);
}

char_t* Widget::caption(ulong_t* len) const
{
    assert(NULL != handle_);
    int length = GetWindowTextLength(handle());
    char_t* buffer = StrAlloc<char_t>(length);
    if (NULL == buffer)
        return NULL;

    length = GetWindowText(handle_, buffer, length + 1);
    if (NULL != len)
        *len = length;

    return buffer;
}

bool Widget::setCaption(const char_t* text)
{
    return FALSE != SetWindowText(handle(), text);
}

bool Widget::setCaption(UINT resourceId)
{
    char_t* str = LoadString(resourceId);
    if (NULL == str)
        return false;

    bool res = setCaption(str);
    free(str);
    return res;     
}

Widget::AutoDeleteOption Widget::setAutoDelete(AutoDeleteOption ad)
{
    AutoDeleteOption res = autoDelete_;
    autoDelete_ = ad;
    return res;  
}

#ifndef NDEBUG
HWND Widget::handle() const
{
    assert(valid());
    return handle_;
}
#endif

void Widget::bounds(RECT& out) const
{
    GetWindowRect(handle(), &out);
    HWND p = parentHandle();
    if (NULL != p && 0 != (WS_CHILD & style()))
        ScreenToClient(p, out);
}

ulong_t Widget::height() const
{
    RECT out;
    GetWindowRect(handle(), &out);
    return out.bottom - out.top;
}

ulong_t Widget::width() const
{
    RECT out;
    GetWindowRect(handle(), &out);
    return out.right - out.left;
}


bool Widget::attachControl(HWND wnd, UINT id)
{
    HWND w = GetDlgItem(wnd, id);
    if (NULL == w)
        return false;
    attach(w);
    return true;
}

void Widget::anchor(AnchorOption horiz, int hMargin, AnchorOption vert, int vMargin, RepaintOption r)
{
    Rect parentRect;
    HWND parent = parentHandle();
    if (NULL != parent)
        GetClientRect(parent, &parentRect);
    else 
    {
        ZeroMemory(&parentRect, sizeof(parentRect));
        parentRect.right = GetSystemMetrics(SM_CXSCREEN);
        parentRect.bottom = GetSystemMetrics(SM_CYSCREEN);
    }

    Rect rect;
    bounds(rect);
    if (anchorLeft == horiz) 
    {
        rect.right =  parentRect.width() - hMargin + rect.width();
        rect.left = parentRect.width() - hMargin;
    }
    else if (anchorRight == horiz)
        rect.right = rect.left + parentRect.width() - hMargin;

    if (anchorTop == vert)
    {
        rect.bottom = parentRect.height() - vMargin + rect.height();
        rect.top = parentRect.height() - vMargin;
    }
    else if (anchorBottom == vert)
        rect.bottom = rect.top + parentRect.height() - vMargin;

    setBounds(rect, r); 
}

bool Widget::anchorChild(UINT itemId, AnchorOption horiz, int hMargin, AnchorOption vert, int vMargin, RepaintOption repaint) const
{
    HWND item = child(itemId);
    if (NULL == item)
        return false;

    Rect parentRect;
    innerBounds(parentRect);

    Rect rect;
    GetWindowRect(item, &rect);
    ScreenToClient(handle(), rect);

    if (anchorLeft == horiz) 
    {
        rect.right =  parentRect.width() - hMargin + rect.width();
        rect.left = parentRect.width() - hMargin;
    }
    else if (anchorRight == horiz)
        rect.right = rect.left + parentRect.width() - hMargin;

    if (anchorTop == vert)
    {
        rect.bottom = parentRect.height() - vMargin + rect.height();
        rect.top = parentRect.height() - vMargin;
    }
    else if (anchorBottom == vert)
        rect.bottom = rect.top + parentRect.height() - vMargin;

    return FALSE != MoveWindow(item, rect.left, rect.top, rect.width(), rect.height(), repaint);	
}

bool Widget::setTopLeft(long x, long y, RepaintOption repaint)
{
    RECT r;
    bounds(r);
    r.left = x; r.top = y;
    return setBounds(r, repaint);   
}

bool Widget::setExtent(long w, long h, RepaintOption repaint)
{
    Rect r;
    bounds(r);
    r.setExtent(w, h);
    return setBounds(r, repaint);   
}

bool Widget::setBottomRight(long x, long y, RepaintOption repaint)
{
    RECT r;
    bounds(r);
    r.right = x; r.bottom = y;
    return setBounds(r, repaint);   
}

bool Widget::setLeft(long x, RepaintOption repaint)
{
    RECT r;
    bounds(r);
    r.left = x;
    return setBounds(r, repaint);   
}

bool Widget::setTop(long y, RepaintOption repaint)
{
    RECT r;
    bounds(r);
    r.top = y;
    return setBounds(r, repaint);
}

bool Widget::setRight(long x, RepaintOption repaint)
{
    RECT r;
    bounds(r);
    r.right = x;
    return setBounds(r, repaint);
}

bool Widget::setBottom(long y, RepaintOption repaint)
{
    RECT r;
    bounds(r);
    r.bottom = y;
    return setBounds(r, repaint);
}

bool Widget::setWidth(long w, RepaintOption repaint)
{
    Rect r;
    bounds(r);
    r.setWidth(w);
    return setBounds(r, repaint);
}

bool Widget::setHeight(long h, RepaintOption repaint)
{
    Rect r;
    bounds(r);
    r.setHeight(h);
    return setBounds(r, repaint);
}

bool Widget::move(long x, long y, RepaintOption repaint)
{
    Rect r;
    bounds(r);
    r += Point(x, y);
    return setBounds(r, repaint);   
}


void Widget::setStyle(ulong_t style)
{
    SetWindowLong(handle(), GWL_STYLE, LONG(style));
}

void Widget::modifyStyle(ulong_t add, ulong_t remove)
{
    ulong_t s = style();
    s &= ~remove;
    s |= add;
    setStyle(s);   
}


// ------------------------------------------
// message handlers follow...

long Widget::handleDestroy()
{
    HWND handle = handle_;
    assert(NULL != handle);
    detach();
    ulong res = defaultCallback(WM_DESTROY, 0, 0, handle);
    if (autoDelete == autoDelete_)
        delete this;
    return res;
}

long Widget::handleCreate(const CREATESTRUCT& cs)
{
    return defaultCallback(WM_CREATE, 0, reinterpret_cast<LPARAM>(&cs));
}

long Widget::handleCommand(ushort notify_code, ushort id, HWND sender)
{
    return defaultCallback(WM_COMMAND, MAKEWPARAM(id, notify_code), reinterpret_cast<LPARAM>(sender));
}

long Widget::handleResize(UINT sizeType, ushort width, ushort height)
{
    return defaultCallback(WM_SIZE, sizeType, MAKELPARAM(width, height));
}

long Widget::handlePaint(HDC dc, PAINTSTRUCT* ps)
{
    assert(NULL != dc);
    return defaultCallback(WM_PAINT, (WPARAM)dc, 0);
}

LRESULT Widget::rawHandleCommand(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    ushort notify_code = HIWORD(wParam);
    ushort id = LOWORD(wParam);
    HWND sender_handle = reinterpret_cast<HWND>(lParam);
    return this->handleCommand(notify_code, id, sender_handle);
}

LRESULT Widget::rawHandlePaint(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT l = TRUE;
    HDC dc = (HDC)wParam;
    if (NULL != dc)
    {
        l = handlePaint(dc, NULL);
        return l;
    }
    PAINTSTRUCT ps;
    dc = BeginPaint(handle(), &ps);
    if (NULL != dc)
    {
        l = handlePaint(dc, &ps);
        EndPaint(handle_, &ps);
    }
    return l;
}

LRESULT Widget::rawHandleNotify(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPNMHDR hdr = (LPNMHDR)lParam;
    assert(NULL != hdr); 
    return handleNotify(int(wParam), *hdr); 
}

long Widget::handleNotify(int controlId, const NMHDR& hdr)
{
    return defaultCallback(WM_NOTIFY, WPARAM(controlId), LPARAM(&hdr));
}