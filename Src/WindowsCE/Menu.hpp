#ifndef GUI_MENU_H__
#define GUI_MENU_H__

#include <Debug.hpp>
#include <Utility.hpp>

class Menu: private NonCopyable
{
	HMENU menu_;

public:

#ifndef NDEBUG
	HMENU handle() const;
#else
	HMENU handle() const {return menu_;}
#endif

	bool valid() const {return NULL != menu_;}

	enum AutoDestroyOption {
		autoDestroyNot,
		autoDestroy
	};

	explicit Menu(AutoDestroyOption ad = autoDestroyNot);

	explicit Menu(HMENU m, AutoDestroyOption ad = autoDestroyNot);

	void attach(HMENU m);

	void detach();

	enum MenuType {
		menuBar,
		menuPopup
	};

	bool create(MenuType type);

	void destroy();

	virtual ~Menu();

private:

	AutoDestroyOption autoDestroy_;

}; // class Menu

#endif // GUI_MENU_H__