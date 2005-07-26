#ifndef ARSLEXIS_WIN_TEXT_RENDERER_HPP__
#define ARSLEXIS_WIN_TEXT_RENDERER_HPP__

#include <WindowsCE/Controls.hpp>
#include <Definition.hpp>

class TextRenderer: public Widget {
	
	ScrollBar scrollBar_;
	bool scrollbarVisible_;
	bool leftButtonDown_;
	HGDIOBJ origBitmap_;
	
	void verifyScrollbarVisible(ushort width, ushort height);
	void updateScroller(RepaintOption repaint);
	
	void prepareTestData();
	
	enum ScrollType {
		scrollHome,
		scrollLine,
		scrollPage,
		scrollEnd,
		scrollPosition
	};
	
	void scroll(int units, ScrollType type);
	
	void paintDefinition(HDC dc, int scroll = 0);
	
	HDC prepareOffscreenDC(HDC orig, Rect& rect);
	void updateOrigDC(HDC offs, HDC orig, const Rect& rect);
	
public:

	explicit TextRenderer(AutoDeleteOption ad = autoDeleteNot);

	bool create(DWORD style, int x, int y, int w, int h, HWND parent, HINSTANCE instance);

	Definition definition;
	void setModel(DefinitionModel* model, Definition::ModelOwnerFlag own = Definition::ownModelNot);
	
protected:

	virtual LRESULT callback(UINT uMsg, WPARAM wParam, LPARAM lParam);

	long handleCreate(const CREATESTRUCT& cs);
	long handleResize(UINT sizeType, ushort width, ushort height);
	long handlePaint(HDC dc);
//	long handleDestroy();

private:

	bool mouseAction(int x, int y, UINT clickCount);
	
	LRESULT handleVScroll(UINT msg, WPARAM wParam, LPARAM lParam);
};

#endif