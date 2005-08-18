#include <WindowsCE/WinTextRenderer.hpp>
#include <LineBreakElement.hpp>
#include <TextElement.hpp>
#include "MoriartyStyles.hpp"

using namespace DRA;
static const char_t* textRendererWindowClass = TEXT("TextRenderer");

static ATOM TextRendererClass(HINSTANCE instance)
{
	static ATOM a = Widget::registerClass(CS_DBLCLKS|CS_PARENTDC, instance, NULL, NULL, (HBRUSH)(COLOR_WINDOW+1), textRendererWindowClass);
	return a;	
}

TextRenderer::TextRenderer(AutoDeleteOption ad):
	Widget(ad),
	scrollbarVisible_(false),
	leftButtonDown_(false),
	offscreenDC_(NULL),
	origBitmap_(NULL),
	scrollTimer_(0),
	scrollDirection_(scrollNone)
{}

TextRenderer::~TextRenderer()
{
	destroyOffscreenDC();
}

bool TextRenderer::create(DWORD style, int x, int y, int w, int h, HWND parent, HINSTANCE instance)
{
	ATOM a = TextRendererClass(instance);
	if (NULL == a)
		return false;
		
	style |= WS_CHILD;
	return Widget::create(a, NULL, style, x, y, w, h, parent, NULL, instance);
}

long TextRenderer::handleCreate(const CREATESTRUCT& cs)
{
	int sx = GetSystemMetrics(SM_CXVSCROLL);
	if (!scrollBar_.create(SBS_VERT, cs.cx - sx, 0, sx, cs.cy, handle(), cs.hInstance))
		return createFailed;
	
	prepareTestData();
	
	return Widget::handleCreate(cs);
}

void TextRenderer::verifyScrollbarVisible(ushort width, ushort height)
{
	Rect r;
	scrollBar_.bounds(r);
	int sx = r.width() + SCALEX(1);
	
	if (scrollbarVisible_)
		width -= sx;
		
	r.set(0, 0, width, height);
	if (definition.layoutChanged(r))
	{
		Graphics g(handle());
		definition.calculateLayout(g, r);
		bool recalc = false;
		if (scrollbarVisible_ && definition.shownLinesCount() == definition.totalLinesCount())
		{
			scrollbarVisible_ = false;
			scrollBar_.show(SW_HIDE);
			width += sx;
			recalc = true;
		}
		else if (!scrollbarVisible_ && definition.shownLinesCount() < definition.totalLinesCount())
		{
			scrollbarVisible_ = true;
			scrollBar_.show(SW_SHOW);
			width -= sx;
			recalc = true;
		}
		if (recalc)
		{
			r.setWidth(width);
			definition.calculateLayout(g, r);
		}
		if (scrollbarVisible_)
			updateScroller(repaintNot);
	}
}

void TextRenderer::updateScroller(RepaintOption repaint)
{
	SCROLLINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;
	si.nMin = 0;
	
    int first = definition.firstShownLine(); 
    int total = definition.totalLinesCount();
    int shown = definition.shownLinesCount();
    
   assert(total > shown);
	si.nMax = total - 1;
	si.nPage = shown;
	si.nPos = first;
	scrollBar_.setScrollInfo(si, repaint);
}

void TextRenderer::destroyOffscreenDC()
{
	if (NULL != offscreenDC_)
	{
		DeleteObject(SelectObject(offscreenDC_, origBitmap_));
		origBitmap_ = NULL;
		DeleteDC(offscreenDC_);
		offscreenDC_ = NULL;		
	}
}


long TextRenderer::handleResize(UINT sizeType, ushort width, ushort height)
{
	destroyOffscreenDC();
	verifyScrollbarVisible(width, height);

	Rect rect;
	scrollBar_.bounds(rect);
	scrollBar_.anchor(anchorLeft, rect.width(), anchorBottom, 0, repaintWidget);
	return Widget::handleResize(sizeType, width, height);
}

long TextRenderer::handlePaint(HDC dc)
{
	paintDefinition(dc);
	return Widget::handlePaint(dc);
}

void TextRenderer::setModel(DefinitionModel* model, Definition::ModelOwnerFlag own)
{
	definition.setModel(model, own);
	Rect r;
	bounds(r);
	verifyScrollbarVisible(ushort(r.width()), ushort(r.height()));
	InvalidateRect(handle(), NULL, FALSE);
}

void TextRenderer::prepareTestData()
{
	DefinitionModel* model = new_nt DefinitionModel();
	if (NULL == model)
		return;
		
	DefinitionModel::Elements_t& elems = model->elements;
	for (int i = 0; i < 100; ++i)
	{
		elems.push_back(new_nt TextElement(TEXT("Eins zwei drei testing. ")));
		switch (i % 5) {
			case 0:
				elems.back()->setStyle(StyleGetStaticStyle(styleNameHeader));
				break;
			case 1:
				elems.back()->setJustification(DefinitionElement::justifyCenter);
				break;
			case 2:
				elems.back()->setStyle(StyleGetStaticStyle(styleNameBold));
				break;
			case 3:
				elems.push_back(new_nt LineBreakElement());
				break;
			case 4:
				elems.back()->setHyperlink("http://www.google.com", -1, hyperlinkUrl);
				break;
		}
	}
	setModel(model, Definition::ownModel);
	// definition.setInteractionBehavior(Definition::behavDoubleClickSelection|Definition::behavHyperlinkNavigation|Definition::behavMouseSelection|Definition::behavUpDownScroll|Definition::behavSelectionClickAction);
	// definition.setNavOrderOptions(Definition::navOrderFirst|Definition::navOrderLast);
}
	
LRESULT TextRenderer::callback(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_VSCROLL:
			return handleVScroll(uMsg, wParam, lParam);
		
		case WM_LBUTTONDOWN:
			if (handle() != GetFocus())
				SetFocus(handle());
			
			leftButtonDown_ = true;
			if (mouseAction(short(LOWORD(lParam)), short(HIWORD(lParam)), 0))
			{
				SetCapture(handle());
				return messageHandled;
			}
			break;
		
		case WM_MOUSEMOVE:
			if (leftButtonDown_ && mouseAction(short(LOWORD(lParam)), short(HIWORD(lParam)), 0))
				return messageHandled;
			break;
		
		case WM_LBUTTONUP:
			if (leftButtonDown_)
			{
				if (handle() == GetCapture())
					ReleaseCapture();
					
				if (0 != scrollTimer_)
				{
					KillTimer(handle(), scrollTimer_);
					scrollTimer_ = 0;
					scrollDirection_ = scrollNone;
				}
					
				leftButtonDown_ = false;
				if (mouseAction(short(LOWORD(lParam)), short(HIWORD(lParam)), 1))
					return messageHandled;
			}
			break;
		
		case WM_ACTIVATE:
			if (WA_INACTIVE == LOWORD(wParam) && handle() == GetCapture())
				ReleaseCapture();
			return defaultCallback(uMsg, wParam, lParam);
		
		case WM_LBUTTONDBLCLK:
			if (mouseAction(LOWORD(lParam), HIWORD(lParam), 2))
				return messageHandled;
			break;
		
		case WM_KILLFOCUS:
		case WM_SETFOCUS:
			InvalidateRect(handle(), NULL, FALSE); // WM_PAINT handler will determine whether it has focus and draw focus ring appropriately
			break;
		
		case WM_KEYDOWN:
			return handleKeyDown(uMsg, wParam, lParam);
			
		case WM_TIMER:
			return handleTimer(uMsg, wParam, lParam);
			
		case WM_GETDLGCODE:
			return DLGC_WANTARROWS;
	}
	return Widget::callback(uMsg, wParam, lParam);
}

LRESULT TextRenderer::handleVScroll(UINT msg, WPARAM wParam, LPARAM lParam)
{
	int code = LOWORD(wParam);
	HWND wnd = (HWND)lParam;
	assert(scrollBar_.handle() == wnd);
	bool track = false;
	switch (code)
	{
        case SB_TOP:
            if (scroll(0, scrollHome))
				return messageHandled;
			break;
            
        case SB_BOTTOM:
            if (scroll(0, scrollEnd))
				return messageHandled;
			break;

        case SB_LINEUP:
            if (scroll(-1, scrollLine))
				return messageHandled;
			break;

        case SB_LINEDOWN:
            if (scroll(1, scrollLine))
				return messageHandled;
			break;

        case SB_PAGEUP:
            if (scroll(-1, scrollPage))
				return messageHandled;
			break;

        case SB_PAGEDOWN:
            if (scroll(1, scrollPage))
				return messageHandled;
			break;

		case SB_THUMBTRACK:
			track = true;
		// intentional fallthrough
        case SB_THUMBPOSITION:
        {	
			int pos = HIWORD(wParam);
/*
			SCROLLINFO si; 
			ZeroMemory(&si, sizeof(si));
			si.cbSize = sizeof(si);
			if (track)
				si.fMask = SIF_TRACKPOS;
			else
				si.fMask = SIF_POS;
			scrollBar_.getScrollInfo(si);
			if (track)
				pos = si.nTrackPos;
			else
				pos = si.nPos;
 */	
            if (scroll(pos, scrollPosition))
				return messageHandled;
			break;
        }
	}	
	return defaultCallback(msg, wParam, lParam);
}

bool TextRenderer::scroll(int units, ScrollType type, const Point* p)
{
	assert(!definition.empty());
	switch (type) 
	{
		case scrollPage:
			units *= definition.shownLinesCount();
			break;
		
		case scrollEnd:
			units = definition.totalLinesCount();
			break;
		
		case scrollHome:
			units = -int(definition.totalLinesCount());
			break;
		
		case scrollPosition:
			units -= definition.firstShownLine();
			break;
	}

	bool res = false;	
	if (0 != units)
	{
		HDC dc = GetWindowDC(handle());
		if (NULL != dc)
		{
			uint_t f = definition.firstShownLine();
			paintDefinition(dc, units, p);
			if (definition.firstShownLine() != f)
			{
				updateScroller(repaintWidget);
				res = true; 
			}
			ReleaseDC(handle(), dc);
		}
	}
	return res;
}

void TextRenderer::definitionBounds(Rect& rect)
{
	bounds(rect);
	if (scrollbarVisible_)
	{
		Rect sr;
		scrollBar_.bounds(sr);
		int sx = sr.width() + SCALEX(1);
		rect.setWidth(rect.width() - sx);
	}
	
	rect.set(SCALEX(1), SCALEY(1), rect.width() - SCALEX(2), rect.height() - SCALEY(2));
}


HDC TextRenderer::prepareOffscreenDC(HDC orig, Rect& rect)
{
	if (NULL != offscreenDC_)
		return offscreenDC_;
		
	HDC dc = 	CreateCompatibleDC(orig);
	if (NULL == dc)
		return NULL;
	
	definitionBounds(rect);
    HBITMAP bitmap = CreateCompatibleBitmap(orig, rect.width() + SCALEX(2), rect.height() + SCALEY(2));
	if (NULL == bitmap)
	{
		DeleteDC(dc);
		return NULL;
	} 
	
	assert(NULL == origBitmap_);
	origBitmap_ = SelectObject(dc, bitmap);
	BitBlt(dc, 0, 0, rect.width() + SCALEX(2), rect.height() + SCALEY(2), orig, 0, 0, SRCCOPY);
	offscreenDC_ = dc;
	return dc;
}

void TextRenderer::updateOrigDC(HDC orig, Rect& rect)
{
	assert(NULL != offscreenDC_);
	assert(NULL != orig);
	
	rect.set(0, 0, rect.width() + SCALEX(2), rect.height() + SCALEY(2));

	HGDIOBJ obj = SelectObject(offscreenDC_, CreatePen(PS_SOLID, 1, GetSysColor(COLOR_WINDOW)));
	Point p[5] = {
		rect.topLeft(), 
		Point(rect.left, rect.bottom - 1), 
		Point(rect.right - 1, rect.bottom - 1), 
		Point(rect.right - 1, rect.top), 
		rect.topLeft()
	};
	Polyline(offscreenDC_, p, 5);
	DeleteObject(SelectObject(offscreenDC_, obj));
 	
	if (handle() == GetFocus() && (definition.usesDoubleClickSelection() || definition.usesMouseSelection()
		 || definition.usesHyperlinkNavigation() || definition.usesUpDownScroll()))
	{
#if _WIN32_WCE >= 0x500	
		DrawFocusRectColor(offscreenDC_, &rect, DFRC_FOCUSCOLOR);
#else
		DrawFocusRect(offscreenDC_, &rect);
#endif
	}
	
	BitBlt(orig, 0, 0, rect.width(), rect.height(), offscreenDC_, 0, 0, SRCCOPY);
	destroyOffscreenDC();
}


void TextRenderer::paintDefinition(HDC orig, int scroll, const Point* p)
{
	
	Rect r;
	HDC dc = prepareOffscreenDC(orig, r);
	if (NULL == dc)
		return;
	{	
		Graphics g(dc, Graphics::deleteNot);
		if (0 == scroll)
			definition.render(g, r);
		else
			definition.scroll(g, scroll);
		if (NULL != p)
			definition.extendSelection(g, *p);
	}
	updateOrigDC(orig, r);
}

bool TextRenderer::mouseAction(int x, int y, UINT clickCount)
{
	if (definition.empty())
		return false;
		
	Rect r;
	definitionBounds(r);
	Point p(x - r.x(), y - r.y());
		
	if (p && r)
	{
		scrollDirection_ = scrollNone;
		if (0 != scrollTimer_)
		{
			KillTimer(handle(), scrollTimer_);
			scrollTimer_ = 0;
		}
	}
	else if (0 == clickCount)
	{
		if (p.y < r.y())
			scrollDirection_ = scrollUp;
		else if (p.y >= r.y() + r.height())
			scrollDirection_ = scrollDown;
		else 
			scrollDirection_ = scrollNone;
		if (0 == scrollTimer_ && scrollNone != scrollDirection_ && definition.usesMouseSelection())
			scrollTimer_ = SetTimer(handle(), timerId, 1000/7, NULL);
		return true;
	}

	HDC orig = GetWindowDC(handle());
	if (NULL == orig)
		return false;
	
	HDC offs = prepareOffscreenDC(orig, r);
	bool res = false;
	if (NULL != offs)
	{
		{
			Graphics g(offs, Graphics::deleteNot);
			res = definition.extendSelection(g, p, clickCount);
		}
		updateOrigDC(orig, r);
	}
	ReleaseDC(handle(), orig);
	return res;
}

LRESULT TextRenderer::handleKeyDown(UINT msg, WPARAM wParam, LPARAM lParam)
{
	Definition::NavigatorKey key = Definition::NavigatorKey(-1);
	int dir = 0;
	switch (wParam)
	{
		case VK_DOWN:
			key = Definition::navKeyDown;
			dir = 1;
			break;
		
		case VK_UP:
			key = Definition::navKeyUp;
			dir = -1;
			break;
		
		case VK_LEFT:
			key = Definition::navKeyLeft;
			dir = -1;
			break;
			
		case VK_RIGHT:
			key = Definition::navKeyRight;
			dir = 1;
			break;

#ifdef VK_ACTION
		case VK_ACTION:
#endif			
		case VK_RETURN:
			key = Definition::navKeyCenter;
			break;
		
		default:
			return defaultCallback(msg, wParam, lParam);
	}
	
	HDC orig = GetWindowDC(handle());
	if (NULL == orig)
		return messageHandled;
	
	bool res = false;
	Rect r;
	HDC offs = prepareOffscreenDC(orig, r);
	if (NULL != offs)
	{
		{
			Graphics g(offs, Graphics::deleteNot);
			res = definition.navigatorKey(g, key);
		}
		updateOrigDC(orig, r);
	}
	ReleaseDC(handle(), orig);
	if (res)
	{
		updateScroller(repaintWidget);
		return messageHandled;
	}
	else if (0 != dir)
	{
		HWND parent = parentHandle();
		if (NULL != parent)
			PostMessage(parent, WM_NEXTDLGCTL, 1 == dir ? 0 : 1, FALSE);
	}
	return defaultCallback(msg, wParam, lParam);	
}

LRESULT TextRenderer::handleTimer(UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (timerId != wParam)
		return defaultCallback(msg, wParam, lParam);
	
	if (scrollNone != scrollDirection_)
	{
		Rect r;
		definitionBounds(r);
		Point p = r.topLeft();

		int unit = -1;
		if (scrollDown == scrollDirection_)
		{
			unit = 1;
			p = r.bottomRight_();
		}
		scroll(unit, scrollLine, &p);
	}
	return messageHandled;
}
