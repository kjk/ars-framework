#ifndef ARSLEXIS_CONTROLS_HPP__
#define ARSLEXIS_CONTROLS_HPP__

#include <WindowsCE/Widget.hpp>
#include <commctrl.h>

#define WINDOW_CLASS_SCROLLBAR TEXT("SCROLLBAR")
#define WINDOW_CLASS_EDITBOX TEXT("EDIT")
#define WINDOW_CLASS_TABCONTROL WC_TABCONTROL
#define WINDOW_CLASS_LISTVIEW WC_LISTVIEW
#define WINDOW_CLASS_BUTTON  TEXT("BUTTON")
#define WINDOW_CLASS_LISTBOX TEXT("LISTBOX")
#define WINDOW_CLASS_COMBOBOX TEXT("COMBOBOX")
#define WINDOW_CLASS_STATIC TEXT("STATIC")
#define WINDOW_CLASS_TRACKBAR TEXT("TRACKBAR")

#define DECLARE_CONTROL_CTORS(ControlClass) \
    explicit ControlClass(AutoDeleteOption ad = autoDeleteNot); \
    explicit ControlClass(HWND wnd, AutoDeleteOption ad = autoDeleteNot); \
    ~ControlClass()

#define DECLARE_CONTROL_CREATE() \
    bool create(DWORD style, const RECT& rect, HWND parent, HINSTANCE inst = NULL, UINT controlId = 0); \
    bool create(DWORD style, int x, int y, int width, int height, HWND parent, HINSTANCE instance = NULL, UINT controlId = 0)

#define DECLARE_CONTROL_ALL(ControlClass) \
    DECLARE_CONTROL_CTORS(ControlClass); \
    DECLARE_CONTROL_CREATE()

class ScrollBar: public Widget {
public:

    DECLARE_CONTROL_ALL(ScrollBar);

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

    DECLARE_CONTROL_CTORS(EditBox);
    
    bool create(DWORD style, int x, int y, int width, int height, HWND parent, HINSTANCE instance = NULL, const char_t* text = NULL);

    bool create(DWORD style, const RECT& r, HWND parent, HINSTANCE instance = NULL, const char_t* text = NULL);

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

    void setSelection(ulong_t start = 0, long end = -1) {sendMessage(EM_SETSEL, start, end);}

    void selection(ulong_t& start, ulong_t& end) const {sendMessage(EM_GETSEL, (WPARAM)&start, (LPARAM)&end);}
};


class ProgressBar: public Widget {
public:
    
    DECLARE_CONTROL_ALL(ProgressBar);

    ulong_t position() const {return sendMessage(PBM_GETPOS, 0, 0);}

    void setRange(ulong_t rangeMin, ulong_t rangeMax) {sendMessage(PBM_SETRANGE32, rangeMin, rangeMax);}

    void setPosition(ulong_t pos) {sendMessage(PBM_SETPOS, pos, 0);}

};

class TabControl: public Widget {

public:

    DECLARE_CONTROL_ALL(TabControl);

    HIMAGELIST imageList() const {return TabCtrl_GetImageList(handle());} 

    HIMAGELIST setImageList(HIMAGELIST lst) {return TabCtrl_SetImageList(handle(), lst);}

    ulong_t tabCount() const {return TabCtrl_GetItemCount(handle());}

    bool tab(ulong_t index, TCITEM& it) const {return FALSE != TabCtrl_GetItem(handle(), index, &it);}

    bool setTab(ulong_t index, const TCITEM& it) {return FALSE != TabCtrl_SetItem(handle(), index, &it);}

    long insertTab(ulong_t index, const TCITEM& it) {return TabCtrl_InsertItem(handle(), index, &it);}

    bool  removeTab(ulong_t index) {return FALSE != TabCtrl_DeleteItem(handle(), index);}

    bool clear() {return FALSE != TabCtrl_DeleteAllItems(handle());}

    bool tabBounds(ulong_t index, RECT& r) const {return FALSE != TabCtrl_GetItemRect(handle(), index, &r);}

    enum {selectionNone = long(-1)};

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

    enum {tabWidthDefault = long(-1)};
    long setMinTabWidth(long width) {return TabCtrl_SetMinTabWidth(handle(), width);}

    bool setTabHighlight(ulong_t index, bool value) {return FALSE != TabCtrl_HighlightItem(handle(), index, value);}

};

#ifndef LVS_EX_DOUBLEBUFFER
#define LVS_EX_DOUBLEBUFFER 0
#endif

#ifndef LVS_EX_GRADIENT
#define LVS_EX_GRADIENT 0
#endif

class ListView: public Widget {

public:

    DECLARE_CONTROL_ALL(ListView);
    
    bool clear() {return FALSE != ListView_DeleteAllItems(handle());}

    bool removeItem(ulong_t index) {return FALSE != ListView_DeleteItem(handle(), index);}

    bool item(LVITEM& it) const {return FALSE != ListView_GetItem(handle(), &it);}

    ulong_t itemCount() const {return ListView_GetItemCount(handle());}

    long insertItem(const LVITEM& it) {return ListView_InsertItem(handle(), &it);}
    bool setItem(const LVITEM& it) {return FALSE != ListView_SetItem(handle(), &it);}

    HIMAGELIST setImageList(HIMAGELIST list, int type) {return ListView_SetImageList(handle(), list, type);}  

    void setStyleEx(DWORD styleEx) {ListView_SetExtendedListViewStyle(handle(), styleEx);}

    bool setTextBkColor(COLORREF color) {return FALSE != ListView_SetTextBkColor(handle(), color);} 

    void setIconSpacing(ulong_t x, ulong_t y) {ListView_SetIconSpacing(handle(), x, y);}

    void setItemState(ulong_t index, ulong_t data, ulong_t mask) {ListView_SetItemState(handle(), index, data, mask);}

    void redrawItems(ulong_t begin, ulong_t end) {ListView_RedrawItems(handle(), begin, end);}   

    void setColumnWidth(ulong_t index, ulong_t width) {ListView_SetColumnWidth(handle(), index, width);}

    long selection() const {return ListView_GetSelectionMark(handle());}
    long setSelection(long index) {return ListView_SetSelectionMark(handle(), index);}
    long hitTest(LVHITTESTINFO& hti) const {return ListView_HitTest(handle(), &hti);}
    long itemBounds(ulong_t index, RECT& r, int code) const {return ListView_GetItemRect(handle(), index, &r, code);}
    long insertColumn(ulong_t index, const LVCOLUMN& col) {return ListView_InsertColumn(handle(), index, &col);} 

    void focusItem(ulong_t index) {ListView_SetItemState(handle(), index, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);}

};

class Button: public Widget {
public:

    DECLARE_CONTROL_ALL(Button);

    void setCheck(ulong_t state = BST_CHECKED) {sendMessage(BM_SETCHECK, state, 0);} 

    long checked() const {return sendMessage(BM_GETCHECK, 0, 0);}  

};

class ListBox: public Widget {
public:

    DECLARE_CONTROL_ALL(ListBox);
    
};

class ComboBox: public Widget {
public:

    DECLARE_CONTROL_ALL(ComboBox);

    long addString(const char_t* str) {return sendMessage(CB_ADDSTRING, 0, (LPARAM)str);}
    long addString(UINT stringId);
    long insertString(long index, const char_t* str) {return sendMessage(CB_INSERTSTRING, index, (LPARAM)str);}
    long insertString(long index, UINT stringId);
    void clear() {sendMessage(CB_RESETCONTENT, 0, 0);}
    long selection() const {return sendMessage(CB_GETCURSEL, 0, 0);}
    void setSelection(long index) {sendMessage(CB_SETCURSEL, index, 0);}
    long removeString(ulong_t index) {return sendMessage(CB_DELETESTRING, index, 0);}
    long itemCount() const {return sendMessage(CB_GETCOUNT, 0, 0);}
    long droppedWidth() const {return sendMessage(CB_GETDROPPEDWIDTH, 0, 0);} 
    long setDroppedWidth(long w) {return sendMessage(CB_SETDROPPEDWIDTH, w, 0);}
    long itemHeight(long item = 0) const {return sendMessage(CB_GETITEMHEIGHT, item, 0);}
    long setItemHeight(long h, long item = 0) {return sendMessage(CB_SETITEMHEIGHT, item, h);}
    long droppedRect(RECT& r) const {return sendMessage(CB_GETDROPPEDCONTROLRECT, 0, (LPARAM)&r);}
    bool setDroppedRect(const RECT& r);  
};

class TrackBar: public Widget {
public:

    DECLARE_CONTROL_ALL(TrackBar);

    long position() const {return sendMessage(TBM_GETPOS, 0, 0);}
    void setRange(ushort rangeMin, ushort rangeMax, RepaintOption repaint) {sendMessage(TBM_SETRANGE, (WPARAM)(BOOL)repaint, (LPARAM)MAKELONG(rangeMin, rangeMax));}
    void setPosition(long pos, RepaintOption repaint) {sendMessage(TBM_SETPOS, (WPARAM)(BOOL)repaint, (LPARAM)(LONG)pos);}
    
    enum BuddyType {
        buddyRight,
        buddyBottom = buddyRight,
        buddyLeft,
        buddyTop = buddyLeft
    };
    
    HWND setBuddy(BuddyType type, HWND wnd) {return (HWND)sendMessage(TBM_SETBUDDY, (WPARAM)(BOOL)type, (LPARAM)wnd);}
};

typedef TrackBar Slider;

#endif