#include <WindowsCE/Widget.hpp>
#include <WindowsCE/Messages.hpp>

static LRESULT CALLBACK WidgetCallbackDefault(HWND handle, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(handle, uMsg, wParam, lParam);
}

static const tchar* widgetCallbackMagicMessageName = TEXT("WidgetCallbackMagicMessage {8bc96a0b-61d9-4e6d-b21a-685cfe07450d}");

static UINT WidgetCallbackMagicMessage()
{
	static UINT wm = RegisterWindowMessage(widgetCallbackMagicMessageName);
	return wm;
}

static const LRESULT widgetCallbackMagicResponse = 0xf6f42e44;

static bool IsDialog(HWND handle)
{
	char_t buffer[8];
	int len = GetClassName(handle, buffer, 8);
	if (6 == len && 0 ==_tcsicmp(TEXT("DIALOG"), buffer))
		return true;
	return false;
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
	autoDelete_(ad)
{}

Widget::Widget(HWND handle, AutoDeleteOption ad):
	handle_(NULL),
	previousCallback_(NULL),
	autoDelete_(ad)
{
	assert(NULL == fromHandle(handle));
	attach(handle);
}

Widget::~Widget()
{
	detach();
}

void Widget::attach(HWND handle)
{
	if (valid())
		detach();

	assert(NULL != handle);
	handle_ = handle;
	WNDPROC proc = reinterpret_cast<WNDPROC>(GetWindowLong(handle_, GWL_WNDPROC));

	LRESULT res = CallWindowProc(proc, handle, WidgetCallbackMagicMessage(), 0, 0);
	if (widgetCallbackMagicResponse != res)
	{
		WNDPROC newProc = Widget::callback;
		SetWindowLong(handle_, GWL_WNDPROC, reinterpret_cast<LONG>(newProc));
		previousCallback_ = proc;
	}
}

void Widget::detach()
{
	assert(NULL != handle_);
	WNDPROC callback = WidgetCallbackDefault;
	if (NULL != previousCallback_)
		callback = previousCallback_;
		
	SetWindowLong(handle_, GWL_WNDPROC, reinterpret_cast<LONG>(callback));
	SetWindowLong(handle_, GWL_USERDATA, 0);
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
	UINT magicMessage = WidgetCallbackMagicMessage();
	if (uMsg == magicMessage)
		return widgetCallbackMagicResponse;

#ifndef NDEBUG
	const char_t* name = MessageName(uMsg);
	if (NULL != name)
	{
		OutputDebugString(name);
		OutputDebugString(TEXT("\n"));
	}
#endif
		
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
			w->handle_ = handle;
	}
	if (NULL == w)
		return WidgetCallbackDefault(handle, uMsg, wParam, lParam);

	return w->callback(uMsg, wParam, lParam);
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

		default:
			return defaultCallback(message, wParam, lParam);
	};
}

ATOM Widget::registerClass(UINT style, HINSTANCE instance, HICON icon, HCURSOR cursor, HBRUSH brush, LPCTSTR class_name)
{
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

bool Widget::create(ATOM widget_class, LPCTSTR caption, DWORD style, int x, int y, int width, int height, HWND parent, HMENU menu, HINSTANCE instance)
{
	return create(reinterpret_cast<LPCTSTR>(widget_class), caption, style, x, y, width, height, parent, menu, instance);
}

bool Widget::create(LPCTSTR widget_class, LPCTSTR caption, DWORD style, int x, int y, int width, int height, HWND parent, HMENU menu, HINSTANCE instance)
{
	assert(NULL == handle_);
	HWND handle = CreateWindow(widget_class, caption, style, x, y, width, height, parent, menu, instance, this);
	if (NULL == handle)
		return false;
	
	assert(handle_ == handle);
	return true;
}

LRESULT Widget::defaultCallback(UINT uMsg, WPARAM wParam, LPARAM lParam, HWND handle)
{
	if (NULL == handle)
		handle = handle_;
		
	if (NULL != previousCallback_)
		return CallWindowProc(previousCallback_, handle, uMsg, wParam, lParam);
	else
		return DefWindowProc(handle, uMsg, wParam, lParam);
}

/*
void Widget::caption(tstring& out) const
{
	assert(NULL != handle_);
	int length = GetWindowTextLength(handle_);
	out.resize(length);
	length = GetWindowText(handle_, &out[0], length);
	out.resize(length);
}

bool Widget::set_caption(const tchar* text)
{
	assert(NULL != handle_);
	return FALSE != SetWindowText(handle_, text);
}
 */
 
#ifndef NDEBUG
HWND Widget::handle() const
{
	assert(valid());
	return handle_;
}
#endif



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

LRESULT Widget::rawHandleCommand(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ushort notify_code = HIWORD(wParam);
	ushort id = LOWORD(wParam);
	HWND sender_handle = reinterpret_cast<HWND>(lParam);
	return handleCommand(notify_code, id, sender_handle);
}

