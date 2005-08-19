#ifndef ARSLEXIS_CONTROLS_HPP__
#define ARSLEXIS_CONTROLS_HPP__

#include <WindowsCE/Widget.hpp>
#include <commctrl.h>

#define WINDOW_CLASS_SCROLLBAR TEXT("SCROLLBAR")
#define WINDOW_CLASS_EDITBOX TEXT("EDIT")
#define WINDOW_CLASS_TABCONTROL WC_TABCONTROL

class ScrollBar: public Widget {
public:

	explicit ScrollBar(AutoDeleteOption ad = autoDeleteNot);
	
	bool create(DWORD style, int x, int y, int width, int height, HWND parent, HINSTANCE instance);
	
	void setScrollRange(int minPos, int maxPos, RepaintOption repaint) {SetScrollRange(handle(), SB_CTL, minPos, maxPos, repaint);}
	
	void setScrollInfo(const SCROLLINFO& si, RepaintOption repaint) {SetScrollInfo(handle(), SB_CTL, &si, repaint);}
	
	void setScrollPos(int pos, RepaintOption repaint) {SetScrollPos(handle(), SB_CTL, pos, repaint);}
	
	void getScrollInfo(SCROLLINFO& si) const {GetScrollInfo(handle(), SB_CTL, &si);}
	
	//int position() const {return sendMessage(SBM_GETPOS, 0, 0);}
	//
	//void range(int& minPos, int& maxPos) const {sendMessage(SBM_GETRANGE, (WPARAM)&minPos, (LPARAM)&maxPos);}
	
};

class EditBox: public Widget {
public:
	
	explicit EditBox(AutoDeleteOption ad = autoDeleteNot);
	
	bool create(DWORD style, int x, int y, int width, int height, HWND parent, HINSTANCE instance, const char_t* text = NULL, DWORD styleEx = 0);	
	
	bool canUndo() const {return 0 != sendMessage(EM_CANUNDO, 0, 0);}
	
	ulong_t charAtPoint(const Point& p, ulong_t* line = NULL) const;
	
	void emptyUndoBuffer() {sendMessage(EM_EMPTYUNDOBUFFER, 0, 0);}
	
	long lineIndex(long line) const {return sendMessage(EM_LINEINDEX, line, 0);}
	
	long lineFromChar(long charIndex) const {return sendMessage(EM_LINEFROMCHAR, charIndex, 0);}
	
	ulong_t lineCount() const {return sendMessage(EM_GETLINECOUNT, 0, 0);}
	
	ulong_t lineLengthByCharIndex(long charIndex) const {return sendMessage(EM_LINELENGTH, charIndex, 0);}
	
	ulong_t lineLength(long line) const {return lineLengthByCharIndex(lineLength(line));}
	
	void setReadOnly(bool value) {sendMessage(EM_SETREADONLY, value, 0);}
	
	bool readOnly() const {return 0 != (ES_READONLY & style());}
	
	bool multiLine() const {return 0 != (ES_MULTILINE & style());}
	
	bool password() const {return 0 != (ES_PASSWORD & style());}
	
};


class ProgressBar: public Widget {
public:

	explicit ProgressBar(AutoDeleteOption ad = autoDeleteNot);
	
	bool create(DWORD style, int x, int y, int width, int height, HWND parent, HINSTANCE instance);
	
	ulong_t position() const {return sendMessage(PBM_GETPOS, 0, 0);}
	
	void setRange(ulong_t rangeMin, ulong_t rangeMax) {sendMessage(PBM_SETRANGE32, rangeMin, rangeMax);}
	
	void setPosition(ulong_t pos) {sendMessage(PBM_SETPOS, pos, 0);}
	
};

class TabControl: public Widget {
    
public:
    
    explicit TabControl(AutoDeleteOption ad = autoDeleteNot);
   
    explicit TabControl(HWND wnd, AutoDeleteOption ad = autoDeleteNot);
   
   ~TabControl(); 
   
    bool create(DWORD style, int x, int y, int width, int height, HWND parent, HINSTANCE instance, DWORD styleEx = 0);
   
    HIMAGELIST imageList() const {return TabCtrl_GetImageList(handle());} 
    
    HIMAGELIST setImageList(HIMAGELIST lst) {return TabCtrl_SetImageList(handle(), lst);}
   
    ulong_t tabCount() const {return TabCtrl_GetItemCount(handle());}
   
    bool tab(ulong_t index, TCITEM& it) const {return FALSE != TabCtrl_GetItem(handle(), index, &it);}
   
    bool setTab(ulong_t index, const TCITEM& it) {return FALSE != TabCtrl_SetItem(handle(), index, &it);}
   
    long insertTab(ulong_t index, const TCITEM& it) {return TabCtrl_InsertItem(handle(), index, &it);}
   
    bool  removeTab(ulong_t index) {return FALSE != TabCtrl_DeleteItem(handle(), index);}
   
    bool clear() {return FALSE != TabCtrl_DeleteAllItems(handle());}
   
    bool tabBounds(ulong_t index, RECT& r) const {return FALSE != TabCtrl_GetItemRect(handle(), index, &r);}
   
    static const long selectionNone = -1;
    
    long selection() const {return TabCtrl_GetCurSel(handle());} 
   
    long setSelection(ulong_t index) {return TabCtrl_SetCurSel(handle(), index);}
   
    enum AdjustMode {
        adjustControl,
        adjustTab
    };
   
    void adjustBounds(AdjustMode am, RECT& rect) const {TabCtrl_AdjustRect(handle(), am, &rect);}
   
    void setTabSize(ulong_t width, ulong_t height) {TabCtrl_SetItemSize(handle(), width, height);}
   
    void setPadding(ulong_t h, ulong_t v) {TabCtrl_SetPadding(handle(), h, v);}
   
    ulong_t rowCount() const {return TabCtrl_GetRowCount(handle());}
    
    long focusedTab() const {return TabCtrl_GetCurFocus(handle());}
   
    long setFocusedTab(ulong_t index) {return TabCtrl_SetCurFocus(handle(), index);}
   
    static const long tabWidthDefault = -1;
    long setMinTabWidth(long width) {return TabCtrl_SetMinTabWidth(handle(), width);}
   
    bool setTabHighlight(ulong_t index, bool value) {return FALSE != TabCtrl_HighlightItem(handle(), index, value);}
   
      
};
   


#endif