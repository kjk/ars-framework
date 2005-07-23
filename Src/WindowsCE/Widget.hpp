#ifndef GUI_WIDGET_H__
#define GUI_WIDGET_H__

#include <Debug.hpp>
#include <Geometry.hpp>
#include <Utility.hpp>
#include <WindowsCE/Config.hpp>

class Widget: private NonCopyable
{
	UINT lastMessage_;
	WPARAM lastWParam_;
	LPARAM lastLParam_;
	
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

	virtual void detach();

	enum MessageHandlerResult {
		messageHandled = 0,
		createFailed = ulong(-1)
	};

	static ATOM registerClass(UINT style, HINSTANCE instance, HICON icon, HCURSOR cursor, HBRUSH brush, LPCTSTR class_name);

	bool create(ATOM window_class, LPCTSTR caption, DWORD style, int x, int y, int width, int height, HWND parent, HMENU menu, HINSTANCE instance);

	bool create(LPCTSTR window_class, LPCTSTR caption, DWORD style, int x, int y, int width, int height, HWND parent, HMENU menu, HINSTANCE instance);

	bool valid() const {return NULL != handle_;}

	const char_t* caption(ulong_t* length = NULL) const;

	bool setCaption(const char_t* text);

	bool destroy() {return FALSE != DestroyWindow(handle());}

	LRESULT sendMessage(UINT message, WPARAM wParam, LPARAM lParam) {return SendMessage(handle(), message, wParam, lParam);}

	void bounds(ArsRectangle& out) const;

	void innerBounds(ArsRectangle& out) const;

	enum RepaintOption {
		repaintNot,
		repaintWidget
	};

	bool setBounds(const ArsRectangle& rect, RepaintOption repaint = repaintNot); // {return FALSE != MoveWindow(handle(), rect.left, rect.top, rect.width(), rect.height(), repaint);}

	bool update() {return FALSE != UpdateWindow(handle());}

	bool show(int how = SW_SHOW) {return FALSE != ShowWindow(handle(), how);}

	HWND activate() {return SetActiveWindow(handle());}

protected:

	virtual LRESULT callback(UINT uMsg, WPARAM wParam, LPARAM lParam);

	LRESULT defaultCallback(UINT uMsg, WPARAM wParam, LPARAM lParam, HWND handle = NULL);
	
protected: // message handlers

	virtual long handleDestroy();

	virtual long handleCreate(const CREATESTRUCT& cs);

	virtual long handleCommand(ushort notify_code, ushort id, HWND sender);

private: // "raw" message handlers

	LRESULT rawHandleDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam) {return handleDestroy();}

	LRESULT rawHandleCreate(UINT uMsg, WPARAM wParam, LPARAM lParam) {return handleCreate(*reinterpret_cast<LPCREATESTRUCT>(lParam));}

	LRESULT rawHandleCommand(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:

	static LRESULT CALLBACK callback(HWND handle, UINT uMsg, WPARAM wParam, LPARAM lParam);

	friend class Dialog;

	HWND handle_;
	WNDPROC previousCallback_;
	AutoDeleteOption autoDelete_;

};

#endif