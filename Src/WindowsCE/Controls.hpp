#ifndef ARSLEXIS_CONTROLS_HPP__
#define ARSLEXIS_CONTROLS_HPP__

#include <WindowsCE/Widget.hpp>

#define WINDOW_CLASS_SCROLLBAR TEXT("SCROLLBAR")

class ScrollBar: public Widget {
public:

	ScrollBar(AutoDeleteOption ad = autoDeleteNot);
	
	bool create(DWORD style, int x, int y, int width, int height, HWND parent, HINSTANCE instance);
	
	void setScrollRange(int minPos, int maxPos, RepaintOption repaint) {SetScrollRange(handle(), SB_CTL, minPos, maxPos, repaint);}
	
	void setScrollInfo(const SCROLLINFO& si, RepaintOption repaint) {SetScrollInfo(handle(), SB_CTL, &si, repaint);}
	
	void setScrollPos(int pos, RepaintOption repaint) {SetScrollPos(handle(), SB_CTL, pos, repaint);}
	
	void getScrollInfo(SCROLLINFO& si) const {GetScrollInfo(handle(), SB_CTL, &si);}
	
	//int position() const {return sendMessage(SBM_GETPOS, 0, 0);}
	//
	//void range(int& minPos, int& maxPos) const {sendMessage(SBM_GETRANGE, (WPARAM)&minPos, (LPARAM)&maxPos);}
	
};

#endif