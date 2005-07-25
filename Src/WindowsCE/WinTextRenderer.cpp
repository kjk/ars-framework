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
	scrollbarVisible_(false)
{}

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
	//if (shown == total)
	//{
	//	si.nMax = 0;
	//	si.nPage = 0;
	//}
	//else
	//{
		si.nMax = total - 1;
		si.nPage = shown;
	//}
	si.nPos = first;
	scrollBar_.setScrollInfo(si, repaint);
}


long TextRenderer::handleResize(UINT sizeType, ushort width, ushort height)
{
	verifyScrollbarVisible(width, height);
	
	Rect sr;
	scrollBar_.bounds(sr);
	scrollBar_.anchor(anchorLeft, sr.width(), anchorBottom, 0, repaintWidget);
	return Widget::handleResize(sizeType, width, height);
}

long TextRenderer::handlePaint(HDC dc)
{
	{
		Graphics g(dc, Graphics::deleteNot);
		paintDefinition(g);
	}
	return Widget::handlePaint(dc);
}

void TextRenderer::setModel(DefinitionModel* model, Definition::ModelOwnerFlag own)
{
	definition.setModel(model, own);
	Rect r;
	bounds(r);
	verifyScrollbarVisible(ushort(r.width()), ushort(r.height()));
}

void TextRenderer::prepareTestData()
{
	DefinitionModel* model = new_nt DefinitionModel();
	if (NULL == model)
		return;
		
	DefinitionModel::Elements_t& elems = model->elements;
	for (int i = 0; i < 100; ++i)
	{
		elems.push_back(new_nt TextElement(TEXT("Eins zwei drei testing.")));
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
				elems.back()->setHyperlink("test: 1", -1, hyperlinkUrl);
				elems.push_back(new_nt LineBreakElement());
				break;
		}
	}
	setModel(model, Definition::ownModel);	
}
	
LRESULT TextRenderer::callback(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_VSCROLL:
			return handleVScroll(uMsg, wParam, lParam);
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
            scroll(0, scrollHome);
            return messageHandled;
            
        case SB_BOTTOM:
            scroll(0, scrollEnd);
            return messageHandled;

        case SB_LINEUP:
            scroll(-1, scrollLine);
            return messageHandled;

        case SB_LINEDOWN:
            scroll(1, scrollLine);
            return messageHandled;

        case SB_PAGEUP:
            scroll(-1, scrollPage);
            return messageHandled;

        case SB_PAGEDOWN:
            scroll(1, scrollPage);
            return messageHandled;

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
            scroll(pos, scrollPosition);
            return messageHandled;
        }
	}	
	return defaultCallback(msg, wParam, lParam);
}

void TextRenderer::scroll(int units, ScrollType type)
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
	
/*	
	Rect sr;
	scrollBar_.bounds(sr);
	int sx = sr.width() + SCALEX(1);
	
	Rect r;
	bounds(r);
	if (scrollbarVisible_)
		r.setWidth(r.width() - sx);		
 */
 
	Graphics g(handle());
	if (0 != units)
	{
		paintDefinition(g, units);
		updateScroller(repaintWidget);
	}
}

void TextRenderer::paintDefinition(Graphics& gr, int scroll)
{
	Rect sr;
	scrollBar_.bounds(sr);
	int sx = sr.width() + SCALEX(1);
	
	Rect r;
	bounds(r);
	if (scrollbarVisible_)
		r.setWidth(r.width() - sx);
	
	
	HDC dc = 	CreateCompatibleDC(gr.handle());
	if (NULL == dc)
		return;
	
	Graphics g(dc);
    HBITMAP bitmap = CreateCompatibleBitmap(gr.handle(), r.width(), r.height());
	if (NULL == bitmap)
		return;

    HBITMAP oldBitmap=(HBITMAP)SelectObject(dc, bitmap);
    
	Rect rr(0, 0, r.width(), r.height());
    gr.copyArea(r, g, rr.topLeft());

	if (0 == scroll)		
		definition.render(g, rr);
	else
		definition.scroll(g, scroll);
	
	g.copyArea(rr, gr, r.topLeft());
    SelectObject(dc, oldBitmap);
    DeleteObject(bitmap);
}
