#include <WindowsCE/Menu.hpp>

Menu::Menu(AutoDestroyOption ad):
	menu_(NULL),
	autoDestroy_(ad)
{
}

Menu::Menu(HMENU m, AutoDestroyOption ad):
	menu_(NULL),
	autoDestroy_(ad)
{
	attach(m);
}

#ifndef NDEBUG
HMENU Menu::handle() const 
{
	assert(valid());
	return menu_;
}
#endif

void Menu::attach(HMENU m)
{
	if (valid())
		detach();
	menu_ = m;
}

void Menu::detach()
{
	if (valid())
	{
		if (autoDestroy == autoDestroy_)
			destroy();
		menu_ = NULL;
	}
}

void Menu::destroy()
{
	DestroyMenu(handle());
}

Menu::~Menu()
{
	detach();
}

bool Menu::create(MenuType type)
{
	if (valid())
		detach();
	if (menuBar == type)
		menu_ = CreateMenu();
	else
		menu_ = CreatePopupMenu();
	return valid();
}

