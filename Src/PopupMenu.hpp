#ifndef ARSLEXIS_POPUP_MENU_HPP__
#define ARSLEXIS_POPUP_MENU_HPP__

#include <Window.hpp>
#include <TextRenderer.hpp>
#include <RenderingPreferences.hpp>

namespace ArsLexis
{

class Form;

class PopupMenu /*: public Window */
{

    RenderingPreferences prefs_;
    bool running_;
    Rectangle bounds_;
    
public:

    PopupMenu(Form& form);
    
    ~PopupMenu();

    TextRenderer renderer;
    
    void close();
    
    Err run(UInt16 id, const Rectangle& rect);
    
    bool handleEventInForm(EventType& event);
    
    void draw();

};

}

#endif // ARSLEXIS_POPUP_MENU_HPP__