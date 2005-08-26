#ifndef GUI_COMMAND_BAR_H__
#define GUI_COMMAND_BAR_H__

#include <WindowsCE/Widget.hpp>

#ifdef SHELL_MENUBAR

#include <WindowsCE/Menu.hpp>

class CommandBar: public Widget
{
public:
	
	CommandBar();
	
	~CommandBar();

	explicit CommandBar(HWND handle);

	bool create(HWND parent, DWORD flags, UINT menu_id, HINSTANCE instance = NULL, UINT bitmap_id = NULL, UINT bitmap_count = NULL, COLORREF background_color = 0);

	void adjustParentSize();

	HMENU menuHandle() const;

	void attach(HWND handle);

	void detach();

	bool drawMenuBar() {return FALSE != DrawMenuBar(handle());}

	Menu& menuBar() {return menuBar_;}

	const Menu& menuBar() const {return menuBar_;}
	
	HMENU subMenu(UINT id) const;
	
	bool replaceSubMenu(UINT id, UINT newId, HMENU newMenu, UINT captionId);
	
	bool replaceButton(UINT id, UINT newId, UINT captionId);

protected:

private:
	
	Menu menuBar_;

}; // class CommandBar

#endif // SHELL_MENUBAR

#endif // GUI_COMMAND_BAR_H__