#ifndef ARSLEXIS_POPUP_RENDERER_HPP__
#define ARSLEXIS_POPUP_RENDERER_HPP__

#include <TextRenderer.hpp>

class Form;

class PopupRenderer: public TextRenderer
{

    bool running_;
    UInt16 prevFocusIndex_;
    
public:

    PopupRenderer(Form& form);
    
    ~PopupRenderer();

    
    void close();
    
    Err run(UInt16 id, const Rectangle& rect);
    
    bool handleEventInForm(EventType& event);
    
protected:

    void handleDraw(Graphics& graphics);
    
};

#endif // ARSLEXIS_POPUP_MENU_HPP__