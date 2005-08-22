#ifndef GUI_WIDGET_H__
#define GUI_WIDGET_H__

#include <Debug.hpp>
#include <Geometry.hpp>
#include <Utility.hpp>
#include <WindowsCE/Config.hpp>

void DumpMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

class Widget: private NonCopyable
{
public:

	enum AutoDeleteOption {
		autoDeleteNot,
		autoDelete
	};

	explicit Widget(AutoDeleteOption ad = autoDeleteNot);

	explicit Widget(HWND handle, AutoDeleteOption ad = autoDeleteNot);

	enum CreateOption {
		createNot,
		createWidget
	};

	static Widget* fromHandle(HWND handle, CreateOption co = createNot);

#ifdef NDEBUG
	HWND handle() const {return handle_;}
#else
	HWND handle() const;
#endif

	virtual ~Widget();

	virtual void attach(HWND handle);
	
	bool attachControl(HWND wnd, UINT id);

	virtual void detach();

	enum MessageHandlerResult {
		messageHandled = 0,
		createFailed = ulong(-1)
	};

	static ATOM registerClass(UINT style, HINSTANCE instance, HICON icon, HCURSOR cursor, HBRUSH brush, LPCTSTR class_name);

	bool create(ATOM window_class, LPCTSTR caption, DWORD style, int x, int y, int width, int height, HWND parent, HMENU menu, HINSTANCE instance, DWORD exStyle = 0);

	bool create(LPCTSTR window_class, LPCTSTR caption, DWORD style, int x, int y, int width, int height, HWND parent, HMENU menu, HINSTANCE instance, DWORD exStyle = 0);

	bool valid() const {return NULL != handle_;}

	char_t* caption(ulong_t* length = NULL) const;

	bool setCaption(const char_t* text);

	bool destroy() {return FALSE != DestroyWindow(handle());}

	LRESULT sendMessage(UINT message, WPARAM wParam, LPARAM lParam) const {return SendMessage(handle(), message, wParam, lParam);}

	void bounds(RECT& rect) const;

	void innerBounds(RECT& rect) const {GetClientRect(handle(), &rect);}

	enum RepaintOption {
		repaintNot,
		repaintWidget
	};

	bool setBounds(const RECT& rect, RepaintOption repaint = repaintNot) {return FALSE != MoveWindow(handle_, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, repaint);}

	bool update() {return FALSE != UpdateWindow(handle());}
	
	bool show(int how = SW_SHOW) {return FALSE != ShowWindow(handle(), how);}
	
	bool hide() {return show(SW_HIDE);}
	
	bool isDialog() const;

    bool isChild(HWND parent) const {return FALSE != IsChild(parent, handle());}
	
	bool exists() const {return valid() && FALSE != IsWindow(handle());}
	
	bool visible() const {return FALSE != IsWindowVisible(handle());}
	
	bool enabled() const {return FALSE != IsWindowEnabled(handle());}
	
    void setEnabled(bool state = true) {EnableWindow(handle(), state);}
   
	HWND activate() {return SetActiveWindow(handle());}
	
	HWND parentHandle() const {return GetParent(handle());}
	
    HWND setParent(HWND wnd) {return SetParent(handle(), wnd);} 
	
    enum AnchorOption {
	    anchorNone,
	    anchorLeft,
	    anchorTop = anchorLeft,
	    anchorRight,
	    anchorBottom = anchorRight
    };

	void anchor(AnchorOption horiz, int hMargin, AnchorOption vert, int vMargin, RepaintOption = repaintNot);
	
	bool anchorChild(UINT itemId, AnchorOption horiz, int hMargin, AnchorOption vert, int vMargin, RepaintOption = repaintNot) const;
	
	HWND child(UINT itemId) const {return GetDlgItem(handle(), itemId);}
	
	ulong_t style() const {return GetWindowLong(handle(), GWL_STYLE);}
	ulong_t styleEx() const {return GetWindowLong(handle(), GWL_EXSTYLE);}
	
	void setStyle(ulong_t style);
	void modifyStyle(ulong_t add, ulong_t remove);
	void addStyle(ulong_t style) {modifyStyle(style, 0);}
	void removeStyle(ulong_t style) {modifyStyle(0, style);}

protected:

	virtual LRESULT callback(UINT uMsg, WPARAM wParam, LPARAM lParam);

	LRESULT defaultCallback(UINT uMsg, WPARAM wParam, LPARAM lParam, HWND handle = NULL);
	
protected: // message handlers

	virtual long handleCommand(ushort notify_code, ushort id, HWND sender);

	virtual long handleDestroy();

	virtual long handleCreate(const CREATESTRUCT& cs);
	
	virtual long handleResize(UINT sizeType, ushort width, ushort height);
	
	virtual long handlePaint(HDC dc);
	
    virtual long handleNotify(int controlId, const NMHDR& header);
	
    AutoDeleteOption setAutoDelete(AutoDeleteOption ad);

private: // "raw" message handlers

	LRESULT rawHandleDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam) {return handleDestroy();}

	LRESULT rawHandleCreate(UINT uMsg, WPARAM wParam, LPARAM lParam) {return handleCreate(*reinterpret_cast<LPCREATESTRUCT>(lParam));}

	LRESULT rawHandleCommand(UINT uMsg, WPARAM wParam, LPARAM lParam);
	
	LRESULT rawHandleResize(UINT uMsg, WPARAM wParam, LPARAM lParam) {return handleResize(wParam, LOWORD(lParam), HIWORD(lParam));}
	
	LRESULT rawHandlePaint(UINT uMsg, WPARAM wParam, LPARAM lParam);
	
	LRESULT rawHandleNotify(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:

	static LRESULT CALLBACK callback(HWND handle, UINT uMsg, WPARAM wParam, LPARAM lParam);

	friend class Dialog;
	friend class CommandBar;
	
	HWND handle_;
	WNDPROC previousCallback_;
	LONG previousUserData_;
	AutoDeleteOption autoDelete_;
	
};

BOOL ScreenToClient(HWND wnd, RECT& rect);
BOOL ClientToScreen(HWND wnd, RECT& rect);
inline LONG RectWidth(const RECT& rect) {return rect.right - rect.left;}
inline LONG RectHeight(const RECT& rect) {return rect.bottom - rect.top;}
bool IsDialog(HWND wnd);

#endif