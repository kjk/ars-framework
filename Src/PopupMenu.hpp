#ifndef ARSLEXIS_POPUP_MENU_HPP__
#define ARSLEXIS_POPUP_MENU_HPP__

#include <TextRenderer.hpp>
#include <RenderingPreferences.hpp>

namespace ArsLexis
{

class Form;

class PopupMenu: public TextRenderer
{

    RenderingPreferences prefs_;
    bool running_;
    UInt16 prevFocusIndex_;
    
public:

    PopupMenu(Form& form, RenderingPreferences& prefs);
    
    ~PopupMenu();

    
    void close();
    
    Err run(UInt16 id, const Rectangle& rect);
    
    bool handleEventInForm(EventType& event);
    
protected:

    void handleDraw(Graphics& graphics);
    
};

}

#endif // ARSLEXIS_POPUP_MENU_HPP__