#ifndef GUI_WINDOW_H__
#define GUI_WINDOW_H__

#include <WindowsCE/Widget.hpp>

class Window: public Widget {

public:
    
    void setSizeToInputPanel(bool value) {sizeToInputPanel_ = value;}
     
    bool isInputDialog() const {return isInputDialog_;} 

	explicit Window(AutoDeleteOption ad = autoDeleteNot, bool inputDialog = false);

	explicit Window(HWND handle, AutoDeleteOption ad = autoDeleteNot, bool inputDialog = false);

	void close() {sendMessage(WM_CLOSE, 0, 0);}

protected:

	virtual long handleClose();

	virtual long handleActivate(ushort action, bool minimized, HWND previous);

	virtual long handleSettingChange(ulong flag, LPCTSTR section_name);
	
	virtual long handleExtendedEvent(LPARAM& event);
	
	virtual void handleScreenSizeChange(ulong_t width, ulong_t height);
	
protected:

	LRESULT callback(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:

	LRESULT rawHandleClose(UINT uMsg, WPARAM wParam, LPARAM lParam) {return handleClose();}

	LRESULT rawHandleActivate(UINT uMsg, WPARAM wParam, LPARAM lParam);

	LRESULT rawHandleSettingChange(UINT uMsg, WPARAM wParam, LPARAM lParam) {return handleSettingChange(wParam, reinterpret_cast<LPCTSTR>(lParam));}
	
private:

#ifdef SHELL_AYGSHELL
	SHACTIVATEINFO activateInfo_;
#endif
	bool isInputDialog_;
	bool sizeToInputPanel_;

};

#endif