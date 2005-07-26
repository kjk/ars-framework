#ifndef ARSLEXIS_WIN_TEXT_RENDERER_HPP__
#define ARSLEXIS_WIN_TEXT_RENDERER_HPP__

#include <WindowsCE/Controls.hpp>
#include <Definition.hpp>

class TextRenderer: public Widget {
	
	ScrollBar scrollBar_;
	bool scrollbarVisible_;
	bool leftButtonDown_;
	UINT scrollTimer_;
	
	enum ScrollDirection {
		scrollNone,
		scrollDown,
		scrollUp
	};
	
	enum {timerId = 1};
	
	ScrollDirection scrollDirection_;
	
	HDC offscreenDC_;
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
	
	bool scroll(int units, ScrollType type, const Point* p = NULL);
	
	void paintDefinition(HDC dc, int scroll = 0, const Point* p = NULL);
	void definitionBounds(Rect& r);
	HDC prepareOffscreenDC(HDC orig, Rect& rect);
	void updateOrigDC(HDC orig, Rect& rect);
	
	void destroyOffscreenDC();
	
public:

	explicit TextRenderer(AutoDeleteOption ad = autoDeleteNot);
	
	~TextRenderer();

	bool create(DWORD style, int x, int y, int w, int h, HWND parent, HINSTANCE instance);

	Definition definition;
	void setModel(DefinitionModel* model, Definition::ModelOwnerFlag own = Definition::ownModelNot);
	
protected:

	virtual LRESULT callback(UINT uMsg, WPARAM wParam, LPARAM lParam);

	long handleCreate(const CREATESTRUCT& cs);
	long handleResize(UINT sizeType, ushort width, ushort height);
	long handlePaint(HDC dc);

private:

	bool mouseAction(int x, int y, UINT clickCount);
	
	LRESULT handleVScroll(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT handleKeyDown(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT handleTimer(UINT msg, WPARAM wParam, LPARAM lParam);
};

#endif