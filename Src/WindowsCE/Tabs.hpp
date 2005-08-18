#ifndef ARSLEXIS_TABS_HPP__
#define ARSLEXIS_TABS_HPP__

#include <WindowsCE/Widget.hpp>

class Tab;

class TabContainer: public Widget {
    Widget tabControl_;
   
    bool createContainer(HWND parent, const Rect& rect, HINSTANCE instance); 
    void destroyContainer(); 

public:
    
    explicit TabContainer(AutoDeleteOption ad = autoDeleteNot);
   
    explicit TabContainer(HWND tabControlWnd, AutoDeleteOption ad = autoDeleteNot);
   
   ~TabContainer(); 
   
    void attach(HWND tabControlWnd);
    
    void detach();
   
    bool create(DWORD style, int x, int y, int width, int height, HWND parent, HINSTANCE instance);
   
protected:
        
    LRESULT callback(UINT uMsg, WPARAM wParam, LPARAM lParam);

};
   
       
#endif // ARSLEXIS_TABS_HPP__